# 로그인 및 서버 데이터 동기화 시스템

## 개요

UE5 멀티플레이어 환경에서 닉네임 기반 로그인과 서버-클라이언트 데이터 동기화를 구현한 시스템.  
같은 닉네임으로 접속하면 컴퓨터나 세션이 달라도 항상 동일한 플레이어 데이터를 불러온다.

---

## 핵심 설계 원칙

- **닉네임 → UID**: 동일 닉네임은 항상 동일한 UID. UID를 기준으로 서버 데이터 조회
- **서버 권위(Server Authority)**: 재화·캐릭터 데이터는 서버에서 관리, 클라이언트는 RPC로 수신만
- **로컬 세이브 분리**: 파티 프리셋 등 클라이언트 전용 데이터는 닉네임별 로컬 파일로 저장

---

## 관련 클래스

| 클래스 | 역할 |
|---|---|
| `ABAPlayerController` | 로그인 RPC 전송, 클라이언트 데이터 수신 |
| `ABAGameModeBase` | 닉네임 등록, UID 조회/생성, 서버 데이터 전송 |
| `UBAGameInstance` | ClientTravel 전후 닉네임 임시 보관 |
| `UBAResourceServerSaveGame` | UID→재화 데이터, 닉네임→UID 매핑 영구 저장 |
| `UBACharacterServerSaveGame` | UID→보유 캐릭터 목록 영구 저장 |
| `UBAPartySubsystem` | 닉네임별 파티 프리셋 로컬 저장 |
| `UBAResourceSubsystem` | 클라이언트 측 재화 캐시 및 UI 이벤트 |
| `UBACharacterDataSubsystem` | 클라이언트 측 캐릭터 캐시 |

---

## 로그인 플로우

```
[1] 유저가 닉네임 입력 후 Connect 클릭
        ↓
[2] ABAPlayerController::ConnectToServer()
    - GI->PendingNickname = "Asd"
    - GI->bIsConnectingToServer = true
    - ClientTravel(ServerIP)
        ↓
[3] 클라이언트 재접속 → BeginPlay() 재실행
    - bIsConnectingToServer == true 이므로 LOGIN 화면 스킵
    - SetTimerForNextTick → ServerRegisterNickname("Asd") RPC 전송
        ↓
[4] 서버: ABAGameModeBase::RegisterNicknameForPlayer(PC, "Asd")
    - NicknameToUID["Asd"] 조회 → 없으면 GUID 신규 생성
    - RegisterPlayerUID() → 재화/캐릭터 데이터 조회 or 신규 생성
    - ClientInitPlayerData() RPC 응답
        ↓
[5] 클라이언트: ClientInitPlayerData_Implementation()
    - BAResourceSubsystem::InitializeFromServer()
    - BACharacterDataSubsystem::InitializeFromServer()
    - BAPartySubsystem::LoadForNickname("Asd")  ← 닉네임별 로컬 슬롯 로드
    - BAUIManager::ShowScreen(MAIN)  ← 메인 화면 전환
```

---

## 닉네임 → UID 매핑

`UBAResourceServerSaveGame` 안에 `TMap<FString, FString> NicknameToUID`로 관리.

```
"Asd"  → "a1b2c3d4-..."   (첫 접속 시 GUID 생성 후 저장)
"qwer" → "e5f6g7h8-..."
```

- 같은 닉네임은 항상 같은 UID → 세션이 달라도 동일한 데이터 로드
- UID 신규 생성 즉시 `SaveServerData()` 호출로 디스크에 기록

---

## 서버 세이브 파일

| 슬롯 이름 | 내용 |
|---|---|
| `BA_ResourceSlot_Server` | 닉네임→UID 매핑, UID별 재화·레벨 |
| `BA_CharacterSlot_Server` | UID별 보유 캐릭터 목록 |
| `BA_MailSlot_Server` | 메일별 수령 완료 UID 목록 |

---

## 로컬 세이브 파일 (클라이언트)

파티 프리셋은 서버와 무관하게 클라이언트 로컬에 저장.  
닉네임을 슬롯 이름에 포함해서 계정별로 분리.

```
BA_PartySlot_Asd.sav   ← "Asd" 로그인 시 로드
BA_PartySlot_qwer.sav  ← "qwer" 로그인 시 로드
```

`ClientInitPlayerData` 수신 직후 `BAPartySubsystem::LoadForNickname(UserName)` 호출로 로드 타이밍 보장.

---

## PIE 환경 이슈 및 해결책

### 문제

PIE에서 `ClientTravel("IP?Name=Asd?Login=1")` 호출 시 URL 옵션이 서버까지 전달되지 않음.  
PIE 자체 재연결 메커니즘이 가로채서 서버는 DESKTOP 호스트명만 수신.

### 해결

URL 옵션 대신 **Server RPC**로 닉네임 전달.

```cpp
// 클라이언트 BeginPlay에서
ServerRegisterNickname(NicknameCopy);  // Server RPC

// 서버에서
void ABAGameModeBase::RegisterNicknameForPlayer(PC, Nickname)
```

### 실무와의 차이

실무에서는 별도 Auth 서버에서 발급한 **JWT 토큰**을 URL 옵션으로 전달하고,  
`PreLogin` 단계에서 토큰 서명을 검증해 신원 확인 후 접속 허용.  
이 프로젝트는 Auth 서버 없는 포트폴리오 환경이므로 RPC 방식으로 대체.

---

## RPC 목록

| RPC | 방향 | 용도 |
|---|---|---|
| `ServerRegisterNickname` | Client → Server | 로그인 시 닉네임 전달 |
| `ClientInitPlayerData` | Server → Client | 접속 완료 시 전체 데이터 초기화 |
| `ClientReceiveMail` | Server → Client | 메일 수신 |
| `ServerClaimMailReward` | Client → Server | 메일 보상 수령 요청 |
| `ClientApplyMailReward` | Server → Client | 보상 실제 지급 |
