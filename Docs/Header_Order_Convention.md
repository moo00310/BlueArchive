# 헤더 파일 선언 순서 규칙

프로젝트 전체에서 **가독성**과 **일관성**을 위해 헤더(.h)의 선언 순서를 아래 규칙으로 맞춥니다.

---

## 1. 파일 전체 순서

```
1. 인클루드 (#include)
2. 전방 선언 (forward declarations)
3. USTRUCT / UENUM 등 타입 정의
4. UCLASS 클래스 정의 (public → protected → private)
```

---

## 2. 인클루드

- `#pragma once` 또는 가드 다음에 오는 블록.
- 순서: `CoreMinimal.h` → 엔진/언리얼 헤더 → 프로젝트 헤더 → `*.generated.h`(마지막).
- 전방 선언으로 대체 가능하면 `#include` 대신 전방 선언 사용.

---

## 3. 클래스 내부 (public / protected / private 각각)

**섹션별 공통 순서:**

1. **상수·정적 값**  
   `static constexpr`, `static const`, enum 상수
2. **델리게이트·타입**  
   `DECLARE_DYNAMIC_MULTICAST_DELEGATE_*`, 중첩 타입
3. **생성자·소멸자**  
   생성자, 소멸자, `virtual` 소멸자
4. **함수**  
   - 블루프린트/외부 API (`UFUNCTION(BlueprintCallable)` 등)
   - 순수 조회 (`BlueprintPure`, getter)
   - 그 다음 나머지 public/protected/private 함수
5. **멤버 변수**  
   `UPROPERTY` 등 멤버 변수는 해당 접근 제어 블록의 **맨 아래**에 두기.

**정리:**  
각 접근 제어(public/protected/private) 안에서는  
**상수 → 델리게이트/타입 → 생성/소멸 → 함수 → 멤버 변수** 순서를 유지합니다.

---

## 4. 접근 제어 순서

- **public** → **protected** → **private**
- 외부에 노출되는 것부터, 내부 구현에 가까운 것 순으로 읽히도록 합니다.

---

## 5. 함수 그룹 짓기 (선택)

함수가 많을 때는 역할별로 묶고, 그룹 사이에 한 줄 주석으로 구분해도 됩니다.

- 예: `// --- Party ---`, `// --- Window ---`, `// --- Preview ---`
- 그룹 내에서는 **같은 역할끼리** 모으기 (예: Set/Get, Open/Close).

---

## 6. 요약 체크리스트 (클래스 하나 기준)

| 순서 | 내용 |
|------|------|
| 1 | public: 상수·정적 |
| 2 | public: 델리게이트·타입 |
| 3 | public: 생성자·소멸자 |
| 4 | public: UFUNCTION (API) |
| 5 | public: 멤버 변수 (있다면) |
| 6 | protected: 오버라이드·가상 함수 |
| 7 | protected: 기타 함수 |
| 8 | protected: 멤버 변수 |
| 9 | private: 함수 |
| 10 | private: 멤버 변수 |

---

## 7. 예시 (요약)

```cpp
UCLASS()
class UMyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 1) 상수
	static constexpr int32 MaxCount = 4;

	// 2) 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChanged, int32, Id);
	UPROPERTY(BlueprintAssignable)
	FOnChanged OnChanged;

	// 3) 생성자 (필요 시)

	// 4) 함수 (API → Pure → 기타)
	UFUNCTION(BlueprintCallable) void DoSomething();
	UFUNCTION(BlueprintPure) int32 GetId() const { return Id; }

	// 5) public 멤버 변수 (있다면)

protected:
	void NativeConstruct() override;
	// protected 함수들…
	// protected 멤버 변수

private:
	void HandleSomething();
	// private 함수들…
	// private 멤버 변수
};
```

---

이 규칙을 적용하면 **어디에 무엇이 있는지** 예측하기 쉬워져서 가독성이 올라갑니다.  
새로 작성·수정하는 헤더는 이 순서를 기준으로 정리하면 됩니다.
