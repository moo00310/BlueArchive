# 리팩토링 할 것들

## ✅ 완료

### 2. `BAPlayerController` / `BAUIManager` 구조 분리
- `BAUIManager::BeginPlay()` 앞에 `IsLocalController()` guard 추가 → 서버에서 위젯 생성 차단
- `HandleScreenChanged()` UIManager에서 제거 → BAPlayerController로 이관
- `OnScreenChanged` delegate를 public으로 노출
- `BAPlayerController::OnUIScreenChanged()` 추가 — 화면 전환 시 게임 로직 처리
  - SELECT 이탈 → `ReleasePreviewActor()`
  - MAIN 진입 → `PlayMainLobbyBGM()`
- UIManager include 목록에서 `BAPlayerController`, `BAGameInstance` 제거

### 3. `BAUIManager` 게임 로직 직접 호출 제거
- 2번과 함께 해결됨

### 4, 5. SaveGame 중복 → BASaveGameSubsystem 베이스 클래스
- `MarkDirty()` + debounce + `SaveNow()` 공통화
- `BAResourceSubsystem`, `BACharacterDataSubsystem` 상속으로 변경
- SlotName → `static constexpr` 상수로 변경

### 6. 메일 ID 검색 람다 중복 제거
- `BAMailSubsystem::FindMailById()` 헬퍼 추출

### 7. `FindUIDByController()` O(n) → O(1)
- `BAGameModeBase`에 `ControllerToUID` 역방향 맵 추가
- `Logout()`, `FindUIDByController()` O(1)로 개선

### 8. `GetFirstPlayerController()` 교체
- `BAMailSubsystem::ClaimReward()` → `GetFirstLocalPlayerController()` 로 교체

### 9. 오타 수정
- `BABannerWidget` : `SILDESTATE` → `SLIDESTATE`, `SiledState` → `SlideState`

### 10. 네이밍 불일치
- `BAUIManager` : `curScreenType` → `CurrentScreenType`

---

## 🔴 보류

### 1. `BACharacterDataSubsystem` 분리
현재 규모에서 분리 이득보다 작업량이 커서 보류.
캐릭터/파티 데이터가 논리적으로 연관되어 있고, 500줄 이하로 관리 가능한 수준.

---

## 🟢 코드 품질 (미완료)

### 11. 주석 인코딩 깨짐
한글 주석이 깨진 파일들.
- `BAResourceSubsystem.h` 14줄
- `BAPreviewCharacter.h` 24줄
- `BAPreviewCharacter.cpp` 여러 곳

→ 파일을 UTF-8 (with BOM) 로 재저장 필요.
