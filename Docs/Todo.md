# 할일 목록

## 버그 / 알려진 문제

- [ ] **ClaimedMap 영속성 없음**  
  `BAGameModeBase::ClaimedMap`은 서버 메모리에만 존재하고 디스크에 저장되지 않는다.  
  서버가 재시작되면 수령 이력이 초기화되어 같은 메일 보상을 중복 수령할 수 있다.  
  → 해결 방향: `ClaimedMap`을 `BAServerSaveGame`에 포함시켜 서버 저장/로드 흐름에 편입.

- [ ] **GISubsystem 서버 생성 문제**  
  `BAMailSubsystem` 등 클라이언트 전용 GISubsystem이 데디케이트 서버에도 생성됨.  
  → 해결 방향: 서버 전용 로직은 `UWorldSubsystem` + `ShouldCreateSubsystem`으로 분리.

---

## 기능 구현

- [ ] **로그인 UMG 제작**  
  게임 시작 시 표시할 로그인 화면 위젯 구현.  
  → 흐름: 닉네임 입력 → 서버 접속 URL에 포함 → `PreLogin` 유효성 검사 → `PostLogin` 후 `ServerRegisterUID(UID, 닉네임)` → GameMode 서버 파일 저장.
