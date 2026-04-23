# 서버-클라이언트 데이터 구조 리팩토링

> **대상 브랜치**: `slide-banner-6e947`  
> **작업 시점**: 2026-04-23

---

## 배경 — 어떤 문제가 있었나

PIE에서 "Number of Players = 2"로 실행하면 두 클라이언트가 같은 재화·캐릭터 데이터를 공유하는 버그가 있었다.

**원인**: `BAResourceSubsystem`과 `BACharacterDataSubsystem`이 `UBASaveGameSubsystem`을 상속해 직접 파일을 읽고 썼는데, 저장 슬롯 이름이 두 클라이언트 모두 동일(`BA_ResourceSlot.sav`)했다.

```
클라이언트 A → BA_ResourceSlot.sav  ← 같은 파일
클라이언트 B → BA_ResourceSlot.sav  ←
```

---

## 변경 전 구조

```
[클라이언트 A]                         [클라이언트 B]
BAResourceSubsystem                    BAResourceSubsystem
  └─ BA_ResourceSlot.sav (공유됨!)       └─ BA_ResourceSlot.sav (공유됨!)

BACharacterDataSubsystem               BACharacterDataSubsystem
  └─ BA_CharacterSlot.sav (공유됨!)      └─ BA_CharacterSlot.sav (공유됨!)
  └─ 파티 프리셋도 여기서 저장           └─ 파티 프리셋도 여기서 저장
```

- 재화, 캐릭터, 파티 프리셋이 전부 클라이언트 로컬 파일에 저장됨
- 서버/클라이언트 분리 개념 없음
- PIE 다중 플레이어에서 파일 충돌

---

## 변경 후 구조

데이터를 **책임 주체**에 따라 세 종류로 분리했다.

| 데이터 종류 | 관리 주체 | 저장 파일 | 구분 키 |
|---|---|---|---|
| 재화 + 유저 정보 | 서버 (GameMode) | `BA_ResourceSlot_Server.sav` | UID |
| 보유 캐릭터 | 서버 (GameMode) | `BA_CharacterSlot_Server.sav` | UID |
| 파티 프리셋 | 클라이언트 로컬 | `BA_PartySlot.sav` / `BA_PartySlot_PIE{N}.sav` | 파일 자체 = 1인 분 |

```
[서버 — GameMode]
  BA_ResourceSlot_Server.sav
    ├─ UID_A → { Credit:500, Gem:300, UserName:"Alice" }
    └─ UID_B → { Credit:0,   Gem:300, UserName:"Sensei" }

  BA_CharacterSlot_Server.sav
    ├─ UID_A → { Characters: [CHR_001, CHR_002, ...] }
    └─ UID_B → { Characters: [CHR_001, CHR_002, ...] }

[클라이언트 A]                      [클라이언트 B]
BA_PartySlot.sav                    BA_PartySlot_PIE-1.sav
  └─ PlayerUID: "UID_A"               └─ PlayerUID: "UID_B"
  └─ PartyPresets: [...]              └─ PartyPresets: [...]

BAResourceSubsystem (메모리 캐시)   BAResourceSubsystem (메모리 캐시)
BACharacterDataSubsystem (메모리)   BACharacterDataSubsystem (메모리)
```

---

## 전체 데이터 흐름

```
[게임 시작 — 클라이언트]

  BAPlayerController::BeginPlay()  (IsLocalController() == true인 경우만)
    │
    ├─ BAPartySubsystem::GetPlayerUID()
    │    └─ BA_PartySlot.sav 또는 BA_PartySlot_PIE{N}.sav 로드
    │       (파일 없으면 GUID 신규 생성 → 저장)
    │
    └─ ServerRegisterUID(UID)  ──────────────────────→  [서버]
                                                          │
                                              GameMode::RegisterPlayerUID(PC, UID)
                                                          │
                                              BA_ResourceSlot_Server.sav 에서 UID 조회
                                              BA_CharacterSlot_Server.sav 에서 UID 조회
                                              (없으면 기본값 레코드 생성)
                                                          │
                                              ClientInitPlayerData(Resources, Name, Level, Characters)
                                                          │
    ←───────────────────────────────────────────────────┘
    │
    BAResourceSubsystem::InitializeFromServer()
      → 재화 · 유저 정보 메모리 캐시 채움

    BACharacterDataSubsystem::InitializeFromServer()
      → 보유 캐릭터 메모리 캐시 채움

    UI → GetResource(), GetOwnedCharacter() 등 → 메모리에서 즉시 반환


[메일 보상 수령]

  클라이언트: ServerClaimMailReward(MailId)  ──────→  [서버]
                                                         │
                                             ProcessRewardClaim()
                                               → 서버 재화 레코드 업데이트
                                               → SaveServerData()
                                               → ClientApplyMailReward(MailId, Rewards)
                                                         │
  ←──────────────────────────────────────────────────────┘
  BAMailSubsystem::ApplyRewardsLocally()
    → BAResourceSubsystem 메모리 갱신
    → UI 업데이트


[퇴장]

  GameMode::Logout()
    → SaveServerData()  : BA_ResourceSlot_Server.sav, BA_CharacterSlot_Server.sav 저장
```

---

## 파일별 변경 내역

---

### `BAResourceTypes.h`

**추가**: `FBAResourceEntry` 구조체

```cpp
USTRUCT(BlueprintType)
struct FBAResourceEntry
{
    GENERATED_BODY()
    UPROPERTY() EResourceType ResourceType = EResourceType::Credit;
    UPROPERTY() int32 Amount = 0;
};
```

**이유**: `ClientInitPlayerData` RPC로 재화 배열을 전송할 때 쓸 타입이 필요. `TMap`은 RPC 파라미터로 직접 쓸 수 없어서 배열로 변환하기 위한 래퍼 구조체다.

---

### `BAResourceSubsystem.h/.cpp`

| 항목 | 변경 전 | 변경 후 |
|---|---|---|
| 상속 | `UBASaveGameSubsystem` | `UGameInstanceSubsystem` |
| 파일 I/O | 있음 (`BA_ResourceSlot.sav`) | 없음 (메모리 캐시만) |
| PlayerUID | 여기서 관리 | `BAPartySubsystem`으로 이전 |
| 초기화 방식 | 생성 시 파일 로드 | `InitializeFromServer()` 호출로 채워짐 |

**제거**: `GetSlotName()`, `GetSaveData()`, `CachedSlotName`, `GetPlayerUID()`, DataAsset 로딩 코드  
**추가**: `InitializeFromServer(TArray<FBAResourceEntry>, FString UserName, int32 UserLevel)`

---

### `BACharacterDataSubsystem.h/.cpp`

| 항목 | 변경 전 | 변경 후 |
|---|---|---|
| 상속 | `UBASaveGameSubsystem` | `UGameInstanceSubsystem` |
| 파일 I/O | 있음 (`BA_CharacterSlot.sav`) | 없음 (메모리 캐시만) |
| 파티 프리셋 | 여기서 저장 관리 | `BAPartySubsystem`으로 이전 |
| `SetPartyPreset` 파라미터 | `(int32, TArray, int32 LocalPlayerIndex)` | `(int32, TArray)` |

**제거**: 파티 데이터를 `BAPartySaveGame`에 저장하던 코드, `LocalPlayerIndex` 파라미터, `SaveData` UObject  
**추가**: `InitializeFromServer(TArray<FOwnedCharacter>)`, `GetPartyPreset()` / `SetPartyPreset()` — BAPartySubsystem 리다이렉트 래퍼 (기존 Blueprint 코드 호환용)

---

### `BAPartySubsystem.h/.cpp` (신규)

파티 프리셋과 PlayerUID를 **로컬 파일**에 저장하는 서브시스템. 새로 만든 파일.

**PIE 슬롯 분리 핵심 로직**:

```cpp
// BASaveGameSubsystem::Initialize()에서 OwningGameInstance 기준으로 설정된 UserIndex 사용
FString UBAPartySubsystem::ResolveSlotName() const
{
    return UserIndex == 0
        ? TEXT("BA_PartySlot")
        : FString::Printf(TEXT("BA_PartySlot_PIE%d"), UserIndex);
}
```

`UserIndex`는 베이스 클래스 `BASaveGameSubsystem::Initialize()`에서 다음 방식으로 설정된다:

```cpp
for (const FWorldContext& Context : GEngine->GetWorldContexts())
{
    if (Context.OwningGameInstance == GetGameInstance())  // ← 신뢰도 높은 방법
    {
        UserIndex = Context.PIEInstance;
        break;
    }
}
```

> **왜 `WC.World() == World` 방식을 쓰지 않는가?**  
> 월드 컨텍스트에 UWorld 포인터가 세팅되는 타이밍이 `Initialize()` 시점보다 늦을 수 있다.  
> `OwningGameInstance` 방식은 GameInstance가 생성되는 즉시 세팅되므로 타이밍 문제가 없다.

PIE 슬롯 결과:
- 클라이언트 1 (PIEInstance=0) → `BA_PartySlot.sav`
- 클라이언트 2 (PIEInstance=-1) → `BA_PartySlot_PIE-1.sav`
- 실제 빌드 (에디터 아님) → `BA_PartySlot.sav`

`EnsureLoaded()` 동작:
1. 이미 로드됐으면 즉시 반환 (lazy loading)
2. 슬롯 이름 결정 → 파일 존재하면 로드, 없으면 새 객체 생성
3. PlayerUID가 비어있으면 GUID 신규 생성 → 즉시 저장

---

### `BASaveGameSubsystem.h/.cpp`

**추가**: `Initialize()` 오버라이드에서 PIEInstance → `UserIndex` 설정

```cpp
void UBASaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
#if WITH_EDITOR
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        if (Context.OwningGameInstance == GetGameInstance())
        {
            UserIndex = Context.PIEInstance;
            break;
        }
    }
#endif
}
```

이전에는 `BAResourceSubsystem`에만 이 코드가 있었다. 베이스 클래스로 올려서 `BAPartySubsystem`을 포함한 모든 자식이 자동으로 올바른 `UserIndex`를 갖도록 변경.

---

### `BAServerSaveGame.h` (신규)

서버가 전체 플레이어 데이터를 하나의 파일에 집계 보관하기 위한 SaveGame 클래스들. 새로 만든 파일.

```cpp
// 플레이어 1명의 재화 + 유저 정보
USTRUCT()
struct FBAPlayerResourceRecord
{
    UPROPERTY(SaveGame) TMap<EResourceType, int32> Resources;
    UPROPERTY(SaveGame) FString UserName = TEXT("Sensei");
    UPROPERTY(SaveGame) int32 UserLevel = 1;
};

// 플레이어 1명의 보유 캐릭터 목록
USTRUCT()
struct FBAPlayerCharacterRecord
{
    UPROPERTY(SaveGame) TArray<FOwnedCharacter> Characters;
};

// BA_ResourceSlot_Server.sav
UCLASS()
class UBAResourceServerSaveGame : public USaveGame
{
    UPROPERTY(SaveGame) TMap<FString /*UID*/, FBAPlayerResourceRecord> PlayerData;
};

// BA_CharacterSlot_Server.sav
UCLASS()
class UBACharacterServerSaveGame : public USaveGame
{
    UPROPERTY(SaveGame) TMap<FString /*UID*/, FBAPlayerCharacterRecord> PlayerData;
};
```

이전에는 UID별로 파일이 따로 존재했다 (`BA_ResourceSlot_{UID}.sav`). 지금은 서버 파일 2개에 전체 플레이어 데이터가 집계된다.

> **UID 정리 문제**: TMap에 UID가 계속 누적되므로 나중에 오래된 UID를 삭제하는 로직이 필요할 수 있다 (예: LastLoginAt 타임스탬프 기반 만료). 현재는 미구현.

---

### `BAGameModeBase.h/.cpp`

| 항목 | 변경 전 | 변경 후 |
|---|---|---|
| 서버 데이터 구조 | `TMap<FString, UBAResourceSaveGame*>` (UID별 개별 포인터) | `TObjectPtr<UBAResourceServerSaveGame>` (단일 집계 파일) |
| 저장 방식 | 플레이어별 개별 파일 저장 | 두 파일에 전체 저장 |
| 저장 타이밍 | 플레이어별 이벤트 시 | `Logout()` + 보상 지급 시 |

**제거**: `TMap<FString, UBAResourceSaveGame*>`, `TMap<FString, UBACharacterSaveGame*>`, UID별 개별 로드/저장 코드, forward declaration (`class UBAResourceServerSaveGame` 등)  
**추가**: `#include "Save/BAServerSaveGame.h"`, `LoadServerSaves()`, `SaveServerData()`, `GetOrCreateResourceRecord(UID)`, `GetOrCreateCharacterRecord(UID)`

`GetOrCreateResourceRecord()` 동작:
- UID가 맵에 있으면 기존 레코드 참조 반환
- 없으면 기본값 레코드 생성 후 맵에 추가 → 참조 반환

```cpp
// 기본값 (첫 접속 시)
Credit    = 0
Gem       = 300
Energy    = 60
MaxEnergy = 120
UserName  = "Sensei"
UserLevel = 1
```

---

### `BAPlayerController.h/.cpp`

**추가된 RPC**:

| 방향 | 이름 | 파라미터 | 설명 |
|---|---|---|---|
| Client → Server | `ServerRegisterUID` | `FString UID` | 접속 시 UID 서버 등록 |
| Server → Client | `ClientInitPlayerData` | `Resources, UserName, Level, Characters` | 초기 전체 데이터 전송 |

`BeginPlay()` 변경사항:
- 이전: 각 서브시스템이 각자 파일을 로드해서 자체 초기화
- 이후: `BAPartySubsystem`에서 UID 읽기 → `ServerRegisterUID()` → 서버가 `ClientInitPlayerData`로 응답 → 서브시스템 초기화

---

### `BAPartySaveGame.h`

**추가**: `UPROPERTY(SaveGame) FString PlayerUID`  
**제거**: `Gold` 필드 (재화는 서버가 관리하므로 로컬 저장 불필요)

---

## 설계 원칙 요약

```
서버 = 재화, 캐릭터 보유 여부의 원본 (Source of Truth)
       → 파일 2개에 전체 플레이어 데이터 집계
       → 클라이언트가 RPC로 요청하면 데이터 내려줌

클라이언트 서브시스템 = 서버 데이터의 메모리 캐시
                        → 파일 저장 없음
                        → UI가 읽어가는 창구

로컬 = 파티 프리셋만 저장
       → PIEInstance 기반 슬롯 분리로 다중 PIE 지원
```

---

## 향후 DB 전환 시 변경 포인트

이 구조는 나중에 실제 DB(MySQL, Redis 등)를 붙일 때를 고려해 설계됐다.

| 현재 | DB 전환 후 |
|---|---|
| `BA_ResourceSlot_Server.sav` 파일 | DB 테이블 `player_resources` |
| `GameMode::LoadServerSaves()` | 로그인 API 응답에서 데이터 수신 |
| `GameMode::SaveServerData()` | DB 업데이트 쿼리 |
| `GetOrCreateResourceRecord(UID)` | `SELECT ... WHERE uid = ?` + `INSERT IF NOT EXISTS` |

`BAPlayerController` → `BAGameModeBase` → 저장소 레이어의 인터페이스 형태는 그대로 유지되므로, 저장소 구현체만 파일 I/O에서 DB 쿼리로 교체하면 된다.
