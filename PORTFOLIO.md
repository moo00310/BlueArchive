# BlueArchive - Unreal Engine 5 게임 프로젝트 포트폴리오

## 프로젝트 개요
Unreal Engine 5.7 기반 캐릭터 수집 및 파티 관리 시스템. C++과 Blueprint를 활용한 확장 가능한 아키텍처 구현.

---

## 주요 구현 기능

### 1. 캐릭터 데이터 관리 시스템 (BACharacterDataSubsystem)
- DataTable 기반 정적 데이터 관리 (캐릭터 정보, 스탯, 초상화)
- SaveGame 기반 동적 데이터 관리 (레벨, 경험치, 성급)
- 자동 저장 시스템 (Debounce 타이머로 성능 최적화)
- UGameInstanceSubsystem 패턴으로 전역 접근 가능

### 2. 파티 관리 시스템
- 4개의 파티 프리셋 지원 (프리셋당 3명)
- UI를 통한 직관적인 파티 편집
- 프리셋별 독립적인 저장/로드
- 라디오 버튼 기반 프리셋 전환

### 3. 리소스 관리 시스템 (BAResourceSubsystem)
- 크레딧, 에너지, 젬 등 다양한 리소스 타입 관리
- 델리게이트 기반 실시간 UI 업데이트
- Data Asset 기반 기본값 설정
- 자동 저장 시스템

### 4. UI 관리 시스템 (BAUIManager)
- TSoftClassPtr을 활용한 비동기 화면 로딩
- Fade In/Out 화면 전환 애니메이션
- 화면 전환 상태 머신 구현

### 5. SDF 기반 파라미터형 UMG 패널 시스템
- Material Instance Dynamic을 활용한 런타임 파라미터 제어
- HalfSize, TintStrength, ColorAdd 등 SDF 파라미터 동적 설정
- 재사용 가능한 SDF 위젯 컴포넌트 설계

### 6. UMG 마우스 트레일 이펙트 시스템
- 세그먼트 풀링 시스템으로 성능 최적화
- 동적 세그먼트 생성 및 생명주기 관리
- Material 파라미터를 통한 부드러운 트레일 렌더링

### 7. 캐릭터 리스트 시스템
- UniformGridPanel을 활용한 동적 그리드 생성
- 런타임 위젯 동적 생성
- 스크롤 지원 및 가변 레이아웃

### 8. 세이브 시스템
- 기능별 세이브 파일 분리 (캐릭터/파티/리소스)
- Debounce 타이머를 통한 자동 저장
- Deinitialize 시 안전한 데이터 저장

---

## 기술 스택
- 엔진: Unreal Engine 5.7
- 언어: C++ (주요 로직), Blueprint (UI 및 데이터 설정)
- 아키텍처 패턴: Subsystem Pattern, Component Pattern, Delegate Pattern
- 주요 모듈: UMG, Enhanced Input, Niagara

---

## 어필할 수 있는 핵심 역량

### 시스템 설계 능력
- 서브시스템 패턴을 활용한 모듈화된 구조
- 확장 가능하고 유지보수하기 쉬운 아키텍처

### Unreal Engine 전문성
- 서브시스템, 위젯, 세이브 시스템 등 UE5 핵심 기능 활용
- C++과 Blueprint의 효과적인 조합

### 데이터 관리 능력
- 정적/동적 데이터의 명확한 분리
- DataTable과 SaveGame을 활용한 효율적인 데이터 관리

### UI/UX 구현 능력
- 동적 UI 생성 및 레이아웃 관리
- 사용자 친화적인 인터페이스 설계

### 성능 최적화
- 비동기 로딩으로 프레임 드롭 방지
- Debounce 타이머를 통한 불필요한 저장 방지
- Soft Object Reference를 활용한 메모리 효율성

---

## 프로젝트 구조
```
Source/BlueArchive/
├── Character/          # 캐릭터 관련 구조체 및 데이터
├── SubSystem/          # 게임 서브시스템 (데이터 관리)
├── UI/                 # UI 위젯 클래스
├── Manager/            # 매니저 클래스 (UI 관리)
├── Save/               # 세이브 게임 클래스
├── Data/               # 데이터 에셋
├── Game/               # 게임 인스턴스 및 게임 모드
└── Player/             # 플레이어 컨트롤러
```
