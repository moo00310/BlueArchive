# 오답노트

---

## 언리얼 내부 동작

### SaveGame debounce 패턴
- **틀린 이해**: 몰랐음
- **정답**: 데이터 변경마다 SaveGameToSlot 호출 시 매 프레임 디스크 I/O 발생 → 버벅임.  
  `MarkDirty()` + 1초 debounce로 연속 변경을 1번 저장으로 묶음.  
  `Deinitialize`에서 dirty 체크는 게임 종료 시 마지막 변경분 유실 방지.

### UPROPERTY와 GC
- **틀린 이해**: `ClaimedMap`에 UPROPERTY 없으면 GC 문제
- **정답**: GC는 UObject 포인터만 추적. `TMap<FGuid, TSet<FString>>`은 UObject 없음 → GC 문제 아님.  
  진짜 문제는 **영속성** — 서버 재시작 시 수령 이력 유실.  
  `TObjectPtr`가 들어있는 컨테이너는 UPROPERTY 없으면 GC 문제 맞음.

### CDO (Class Default Object)
- **정답**: 엔진 시작 시 클래스당 1개 생성되는 기본값 템플릿.  
  생성자는 CDO 생성 + 실제 인스턴스 생성 시 두 번 불림.  
  CDO는 월드가 없어서 생성자에서 `GetWorld()` → null.  
  → 생성자에 런타임 로직 넣지 말 것.

---

## 설계 판단

### TWeakObjectPtr 사용 이유
- **틀린 이해**: 순환 참조 방지
- **정답**: 언리얼 GC는 Mark-and-Sweep이라 순환 참조가 메모리 누수를 일으키지 않음.  
  진짜 이유는 **수명 불일치** — GISubsystem이 PlayerController보다 오래 살아서.  
  WeakPtr는 PC 소멸 시 자동으로 `IsValid() = false`.

### BAMailSubsystem 분리 이유
- **틀린 이해**: 생명주기 차이만 말함
- **정답**: 생명주기도 맞지만 핵심은 **관심사 분리**.  
  PlayerController = 네트워크 브릿지(RPC만).  
  BAMailSubsystem = 데이터 보관 + UI 알림.  
  UI가 PC를 직접 참조하면 `GetFirstLocalPlayerController`가 PIE에서 null 날 수 있음.

### USTRUCT vs UCLASS
- **틀린 이해**: 에디터 노출 여부 차이
- **정답**:  
  `USTRUCT` = 값 타입, GC 추적 안 됨, 복사 가능, 데이터 묶음에 사용.  
  `UCLASS` = 참조 타입, GC 관리, NewObject/SpawnActor로 생성, 기능+상태가 있는 오브젝트.

---

## 네트워크

### Reliable vs Unreliable
- **틀린 이해**: 값 유효성 여부로 나뉜다 (WithValidation과 혼동)
- **정답**: 패킷 **전달 보장** 여부.  
  `Reliable` = 반드시 도착, 재전송, 중요 이벤트(보상, 사망).  
  `Unreliable` = 드랍 가능, 자주 보내는 비중요 데이터(위치).  
  `WithValidation` = 별개. 서버에서 RPC 호출 합법성 검사하는 `_Validate` 함수 추가.

### ReplicatedUsing / OnRep
- **틀린 이해**: 몰랐음
- **정답**: `Replicated` = 값만 동기화, 클라에서 아무것도 안 불림.  
  `ReplicatedUsing = OnRep_HP` = 값 동기화 + 클라에서 콜백 자동 호출.  
  UI 갱신, 사운드 재생 등 클라 반응이 필요하면 OnRep 필수.

---

## 언리얼 고유 개념

### Dynamic 델리게이트
- **틀린 이해**: 에디터 노출 여부
- **정답**: Blueprint에서 바인딩 가능하다는 의미. 직렬화 지원.  
  `UPROPERTY(BlueprintAssignable)`과 같이 쓰려면 반드시 Dynamic이어야 함.

### NativeConstruct
- **틀린 이해**: 에디터 실행 시 호출
- **정답**: 런타임에 위젯이 뷰포트에 추가될 때 호출. Actor의 BeginPlay와 같은 역할.  
  에디터 디자인 시 미리보기는 `NativePreConstruct`.

### UPROPERTY 스펙
- **틀린 이해**: `EditDefaultsOnly` = 노출만
- **정답**:  
  `EditAnywhere` = 클래스 기본값 + 레벨 인스턴스 둘 다 편집 가능.  
  `EditDefaultsOnly` = 클래스 기본값에서만 편집, 인스턴스에선 불가.  
  `VisibleAnywhere` = 읽기 전용.

### UPROPERTY(SaveGame)
- **틀린 이해**: 몰랐음
- **정답**: `SaveGameToSlot` 호출 시 이 스펙이 없으면 해당 프로퍼티는 파일에 저장 안 됨.  
  SaveGame 클래스 안에서도 명시적으로 붙여야 직렬화 대상이 됨.

---

## C++

### CreateWeakLambda vs [this] 캡처
- **틀린 이해**: 몰랐음
- **정답**:  
  `[this]` = 날 포인터 캡처, this 소멸 후 접근하면 크래시.  
  `CreateWeakLambda(this, ...)` = 실행 전 IsValid() 자동 체크, 소멸됐으면 람다 실행 안 함.  
  비동기 콜백에서 this 쓸 때는 반드시 CreateWeakLambda.

### 비동기 람다 위험 패턴
- 로컬 변수 참조 캡처 → 함수 종료 후 댕글링 참조
- this 날 포인터 캡처 → Actor 소멸 후 크래시
- FStreamableHandle 미보관 → 에셋 언로드 후 null 접근  
  → 비동기에서는 값 캡처 + CreateWeakLambda + Handle 보관 세트로.
