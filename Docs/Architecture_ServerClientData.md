# 서버/클라이언트 데이터 구조 리팩토링

## 핵심 원칙

| 데이터 종류 | 관리 주체 | 저장 위치 |
|-------------|-----------|-----------|
| 재화 (Resources) | **서버** | `BA_ResourceSlot_{UID}.sav` |
| 보유 캐릭터 (OwnedCharacters) | **서버** | `BA_CharacterSlot_{UID}.sav` |
| 파티 편성 (PartyPresets) | **클라이언트** | `BA_PartySlot.sav` (UserIndex로 PIE 구분) |
| 플레이어 UID | **클라이언트** | `BA_PartySlot.sav` 내부 |

---

## 접속 전체 흐름

```
[클라이언트] BeginPlay
  │
  ├─ BAPartySubsystem::LoadOrCreate()
  │    └─ BA_PartySlot.sav 로드 (없으면 신규 생성 + UID 발급)
  │
  └─ ServerRegisterUID(PartySub->GetPlayerUID())  ← RPC (Client → Server)
       │
       [서버] GameMode::RegisterPlayerUID()
         │
         ├─ GetOrCreateResourceData(UID)
         │    └─ BA_ResourceSlot_{UID}.sav 로드 (없으면 기본값 생성)
         │
         ├─ GetOrCreateCharacterData(UID)
         │    └─ BA_CharacterSlot_{UID}.sav 로드 (없으면 CHR_001~035 기본 생성)
         │
         └─ ClientInitPlayerData(Resources, UserName, UserLevel, Characters)
              │                                     ← RPC (Server → Client)
              [클라이언트] PlayerController::ClientInitPlayerData_Implementation()
                ├─ BAResourceSubsystem::InitializeFromServer()   ← 재화 캐시 채움
                └─ BACharacterDataSubsystem::InitializeFromServer() ← 캐릭터 캐시 채움
```

---

## 메일 보상 수령 흐름

```
[클라이언트] 수령 버튼 클릭
  └─ ServerClaimMailReward(MailId)  ← RPC

[서버] GameMode::ProcessRewardClaim()
  ├─ 유효성 검증 (존재/만료/중복)
  ├─ PlayerResourceData[UID] 업데이트  ← 서버 메모리 + .sav 저장
  └─ ClientApplyMailReward(MailId, Rewards)  ← RPC

[클라이언트] BAMailSubsystem::ApplyRewardsLocally()
  └─ BAResourceSubsystem::AddResource()  ← 캐시 업데이트 + UI 갱신
```

> 핵심: 서버가 먼저 자기 데이터를 갱신하고, 그 후 클라이언트 캐시에 반영 명령을 내림.

---

## 서브시스템 역할 정리

### BAPartySubsystem (신규)
- **상속**: `BASaveGameSubsystem` (파일 I/O 지원)
- **역할**: 파티 프리셋 로컬 저장 + `PlayerUID` 발급/보관
- **파일**: `BA_PartySlot.sav` (PIE에서 UserIndex로 인스턴스 구분)
- **UID 발급**: 최초 실행 시 GUID 생성 → 이후 변경 불가

```
BAPartySubsystem
  ├─ GetPlayerUID()          ← ServerRegisterUID RPC에서 사용
  ├─ GetPartyPreset(index)
  └─ SetPartyPreset(index, ids)
```

### BAResourceSubsystem (변경)
- **상속**: `UGameInstanceSubsystem` (파일 I/O 없음)
- **역할**: 서버에서 받은 재화/유저 정보 메모리 캐시
- **초기화**: `ClientInitPlayerData` RPC → `InitializeFromServer()` 호출
- **변경 시**: 캐시 업데이트 + delegate 브로드캐스트 (UI 갱신용)

```
이전: LoadOrCreate() → BA_ResourceSlot.sav 읽기
이후: InitializeFromServer() → 서버 RPC 데이터로 채움
```

### BACharacterDataSubsystem (변경)
- **상속**: `UGameInstanceSubsystem` (파일 I/O 없음)
- **역할 1**: DataTable 정적 데이터 제공 (캐릭터 이름/스탯 등) — 변경 없음
- **역할 2**: 서버에서 받은 보유 캐릭터 메모리 캐시
- **파티 메서드**: `BAPartySubsystem`으로 리다이렉트 (Blueprint 호환 유지)

```
이전: LoadOrCreate() → BA_CharacterSlot.sav + BA_PartySlot.sav 읽기
이후: InitializeFromServer() → 서버 RPC 데이터로 채움
      GetPartyPreset/SetPartyPreset → BAPartySubsystem에 위임
```

### BAGameModeBase (변경)
- 기존 메일 관리 역할에 **플레이어 데이터 관리** 추가
- `PlayerResourceData`: `TMap<UID, UBAResourceSaveGame*>` — 접속 중 메모리에 보관
- `PlayerCharacterData`: `TMap<UID, UBACharacterSaveGame*>` — 접속 중 메모리에 보관
- **Logout 시** 자동 저장 (`SavePlayerData`)

```
GameMode
  ├─ RegisterPlayerUID()  → 데이터 로드 → ClientInitPlayerData RPC
  ├─ ProcessRewardClaim() → 서버 데이터 갱신 → ClientApplyMailReward RPC
  ├─ Logout()             → SavePlayerData()
  ├─ GetOrCreateResourceData(UID)   → .sav 로드 or 기본값 생성
  ├─ GetOrCreateCharacterData(UID)  → .sav 로드 or 기본값 생성
  └─ SavePlayerData(UID)            → .sav 파일 즉시 저장
```

---

## 저장 파일 구조

```
Saved/SaveGames/
  ├─ BA_PartySlot.sav           ← 클라이언트 1 (UserIndex=1)
  ├─ BA_PartySlot_User2.sav     ← 클라이언트 2 (UserIndex=2)
  │
  ├─ BA_ResourceSlot_{UID-A}.sav  ← 서버가 관리 (UserIndex=0)
  ├─ BA_CharacterSlot_{UID-A}.sav
  ├─ BA_ResourceSlot_{UID-B}.sav
  └─ BA_CharacterSlot_{UID-B}.sav
```

---

## 나중에 DB를 붙이면

`BAGameModeBase`의 `GetOrCreateResourceData` / `SavePlayerData` 두 함수만 교체하면 됩니다.

```
현재: UGameplayStatics::LoadGameFromSlot / SaveGameToSlot
DB 이후: HTTP 요청 or DB 드라이버 호출로 교체
```

서브시스템 API(`GetResource`, `HasOwnedCharacter` 등)와 RPC 구조는 그대로 유지됩니다.

---

## 주의 사항

- **기존 `.sav` 파일** (`BA_ResourceSlot.sav`, `BA_CharacterSlot.sav` 등 구형 파일)은 더 이상 읽지 않음. `Saved/SaveGames/`에서 수동 삭제 권장.
- **서버 미접속 상태**에서는 `BAResourceSubsystem`과 `BACharacterDataSubsystem`의 캐시가 비어 있음. `InitializeFromServer()`가 호출되기 전까지 기본값(0, 빈 배열) 반환.
- **로그인 시스템** 추가 시: `ServerRegisterUID` 전에 인증 과정을 끼워넣고, UID를 서버에서 검증한 후 `RegisterPlayerUID`를 호출하면 됨.
