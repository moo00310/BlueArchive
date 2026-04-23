# 메일 UI 에디터 작업 목록

## 0. 빌드 먼저

언리얼 에디터에서 **Ctrl+Alt+F11** 또는 Live Coding으로 빌드.  
`BAMailItemWidget`, `BAMailBoxWidget` C++ 클래스가 인식되어야 이후 작업 가능.

---

## 1. WBP_MailItemWidget 생성

> 메일 1통을 표시하는 행 위젯

1. Content Browser에서 **우클릭 → User Interface → Widget Blueprint**
2. Parent Class: `BAMailItemWidget`
3. 이름: `WBP_MailItemWidget`

### 필수 위젯 이름 (BindWidgetOptional — 없어도 빌드 OK, 있으면 자동 연결)

| 위젯 타입 | 이름 | 역할 |
|---|---|---|
| Text Block | `Text_Title` | 메일 제목 |
| Text Block | `Text_Body` | 메일 본문 |
| Text Block | `Text_Expires` | 만료 일시 |
| Button | `Button_Claim` | 수령 버튼 (미수령 시 표시) |
| 아무 위젯(Border 등) | `Panel_Claimed` | 수령 완료 표시 (수령 후 표시) |

> `Button_Claim`과 `Panel_Claimed`는 C++에서 가시성을 자동으로 토글합니다.  
> 초기 상태: `Button_Claim` Visible / `Panel_Claimed` Collapsed 로 설정.

---

## 2. WBP_MailBoxWidget 생성

> 메일 수신함 전체 화면

1. Content Browser에서 **우클릭 → User Interface → Widget Blueprint**
2. Parent Class: `BAMailBoxWidget`
3. 이름: `WBP_MailBoxWidget`

### 필수 위젯 이름

| 위젯 타입 | 이름 | 역할 |
|---|---|---|
| Scroll Box | `ScrollBox_Mails` | 메일 목록 |
| Button | `Button_ClaimAll` | 전체 수령 버튼 |
| Text Block | `Text_UnreadCount` | 미수령 수 표시 (예: "미수령 3") |

### Details 패널 설정

- `WBP_MailBoxWidget` 선택 후 Details 패널에서  
  **Mail → Mail Item Widget Class** → `WBP_MailItemWidget` 지정

---

## 3. UIManager에 MAIL 화면 등록

1. 에디터에서 `BP_PlayerController` (또는 UIManager가 있는 BP) 열기
2. `BAUIManager` 컴포넌트 선택
3. Details 패널 → **Map Screen Classes** 항목에 항목 추가
   - Key: `MAIL`
   - Value: `WBP_MailBoxWidget`

---

## 4. MAIN 화면에 메일 버튼 추가 (선택)

> 미수령 메일 뱃지를 MAIN 화면에 달고 싶다면

1. `WBP_MainScreen` 열기
2. 메일 버튼 추가
3. 버튼 클릭 이벤트에서:
   ```
   GetPlayerController → RequestShowScreen(MAIL)
   ```
4. 뱃지 텍스트 갱신:  
   - `BeginPlay`에서 `MailSubsystem → OnNewMailReceived` 바인딩  
   - 이벤트 발생 시 `GetUnclaimedCount()` 호출해 뱃지 숫자 갱신
