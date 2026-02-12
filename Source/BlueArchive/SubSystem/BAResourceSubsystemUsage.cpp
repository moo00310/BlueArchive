// ResourceSubsystem 사용 예시
// 이 파일은 참고용이며 실제로는 삭제해도 됩니다

#include "SubSystem/BAResourceSubsystemUsage.h"

// 실제 사용 예시들

// ============================================
// 1. Actor에서 사용
// ============================================
/*
void AMyActor::BeginPlay()
{
    Super::BeginPlay();
    
    // 서브시스템 가져오기
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            UBAResourceSubsystem* ResourceSubsystem = 
                GameInstance->GetSubsystem<UBAResourceSubsystem>();
            
            if (ResourceSubsystem)
            {
                // 크레딧 추가
                ResourceSubsystem->AddCredit(1000);
                
                // 골드 추가
                ResourceSubsystem->AddGold(500);
                
                // 현재 크레딧 확인
                int32 CurrentCredit = ResourceSubsystem->GetCredit();
                UE_LOG(LogTemp, Log, TEXT("Current Credit: %d"), CurrentCredit);
            }
        }
    }
}
*/

// ============================================
// 2. 위젯에서 사용
// ============================================
/*
void UMyWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // BAUserWidget을 상속했다면 헬퍼 함수 사용 가능
    // UBAResourceSubsystem* ResourceSubsystem = GetSubsystem<UBAResourceSubsystem>();
    
    // 또는 직접
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        UBAResourceSubsystem* ResourceSubsystem = 
            GameInstance->GetSubsystem<UBAResourceSubsystem>();
        
        if (ResourceSubsystem)
        {
            // 리소스 변경 이벤트 구독
            ResourceSubsystem->OnResourceChanged.AddDynamic(
                this, 
                &UMyWidget::OnResourceChanged
            );
            
            // 초기값 표시
            UpdateCreditDisplay(ResourceSubsystem->GetCredit());
        }
    }
}

void UMyWidget::OnResourceChanged(EResourceType ResourceType, int32 NewValue)
{
    if (ResourceType == EResourceType::Credit)
    {
        UpdateCreditDisplay(NewValue);
    }
}
*/

// ============================================
// 3. PlayerController에서 사용
// ============================================
/*
void AMyPlayerController::SomeFunction()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        UBAResourceSubsystem* ResourceSubsystem = 
            GameInstance->GetSubsystem<UBAResourceSubsystem>();
        
        if (ResourceSubsystem)
        {
            // 아이템 구매 시도
            if (ResourceSubsystem->SpendCredit(100))
            {
                // 구매 성공
                UE_LOG(LogTemp, Log, TEXT("Purchase successful!"));
            }
            else
            {
                // 크레딧 부족
                UE_LOG(LogTemp, Warning, TEXT("Not enough credit!"));
            }
        }
    }
}
*/

// ============================================
// 4. GameMode에서 사용
// ============================================
/*
void AMyGameMode::GiveStartingResources()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        UBAResourceSubsystem* ResourceSubsystem = 
            GameInstance->GetSubsystem<UBAResourceSubsystem>();
        
        if (ResourceSubsystem)
        {
            // 시작 리소스 지급
            ResourceSubsystem->SetResource(EResourceType::Credit, 10000);
            ResourceSubsystem->SetResource(EResourceType::Gold, 5000);
            ResourceSubsystem->SetResource(EResourceType::Energy, 100);
        }
    }
}
*/
