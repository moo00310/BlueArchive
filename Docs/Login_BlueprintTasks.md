# 로그인 UMG Blueprint 작업

C++ 백엔드 완료 기준. 아래 순서대로 작업.

---

## 1. WBP_Login 위젯 제작

**경로**: `Content/BP/UI/WBP_Login`

### 필요한 위젯 요소

| 이름 | 타입 | 용도 |
|------|------|------|
| `TB_Nickname` | TextBox | 닉네임 입력 |
| `TB_ServerIP` | TextBox | 서버 IP 입력 (Default: `127.0.0.1`) |
| `Btn_Connect` | Button | 접속 버튼 |

### 버튼 로직

C++(`BALoginWidget.cpp`)에서 처리됨. Blueprint에서 별도 노드 연결 불필요.

- `Btn_Connect` 클릭 → `TB_Nickname` 공백 검증 → `ConnectToServer(Nickname, ServerIP)` 자동 호출
- `TB_ServerIP`가 비어 있으면 `127.0.0.1` 자동 사용

> PIE 테스트 시: `NM_Standalone`이 아니므로 버튼 클릭 없이도  
> 서버가 자동으로 `ClientInitPlayerData`를 보내 MAIN으로 전환됨.

---

## 2. UIManager에 LOGIN 케이스 추가

UIManager에서 스크린별 위젯을 생성/표시하는 Switch 또는 Map에  
`EUIScreen::LOGIN → WBP_Login` 케이스 추가.

기존 MAIN, CONTENTS, SELECT 등이 등록된 방식과 동일하게 추가하면 됨.

---

## 흐름 정리

```
[PIE - 이미 연결된 상태]
BeginPlay → LOGIN 화면 표시
→ 서버 PostLogin 자동 처리 (닉네임→UID 매핑)
→ ClientInitPlayerData 수신
→ MAIN 화면 자동 전환

[패키징 클라이언트 - Standalone]
BeginPlay → LOGIN 화면 표시
→ 유저가 닉네임/IP 입력 후 Btn_Connect 클릭
→ ConnectToServer 호출 → ClientTravel
→ 서버 PreLogin 검증 → PostLogin 처리
→ ClientInitPlayerData 수신
→ MAIN 화면 자동 전환
```
