// ResourceSubsystem 사용 예시
// 이 파일은 참고용이며 실제로는 삭제해도 됩니다

#pragma once

#include "CoreMinimal.h"
#include "SubSystem/BAResourceSubsystem.h"  // ← 필수 헤더
#include "SubSystem/BAResourceTypes.h"      // ← EResourceType enum 사용 시 필요
#include "Engine/GameInstance.h"            // ← GetGameInstance() 사용 시 필요

/**
 * ResourceSubsystem 사용 예시 클래스
 */
class BAResourceSubsystemUsageExample
{
public:
    //// 예시 1: Actor에서 사용
    //void ExampleInActor()
    //{
    //    // GameInstance에서 서브시스템 가져오기
    //    if (UWorld* World = GetWorld())
    //    {
    //        if (UGameInstance* GameInstance = World->GetGameInstance())
    //        {
    //            UBAResourceSubsystem* ResourceSubsystem = 
    //                GameInstance->GetSubsystem<UBAResourceSubsystem>();
    //            
    //            if (ResourceSubsystem)
    //            {
    //                // 크레딧 추가
    //                ResourceSubsystem->AddCredit(1000);
    //                
    //                // 크레딧 확인
    //                int32 Credit = ResourceSubsystem->GetCredit();
    //                
    //                // 크레딧 사용
    //                bool bSuccess = ResourceSubsystem->SpendCredit(500);
    //            }
    //        }
    //    }
    //}
    //
    //// 예시 2: 위젯에서 사용 (BAUserWidget 상속 시)
    //void ExampleInWidget()
    //{
    //    // BAUserWidget의 헬퍼 함수 사용
    //    // UBAResourceSubsystem* ResourceSubsystem = GetSubsystem<UBAResourceSubsystem>();
    //    
    //    // 또는 직접
    //    if (UGameInstance* GameInstance = GetGameInstance())
    //    {
    //        UBAResourceSubsystem* ResourceSubsystem = 
    //            GameInstance->GetSubsystem<UBAResourceSubsystem>();
    //        
    //        if (ResourceSubsystem)
    //        {
    //            // 리소스 변경 이벤트 구독
    //            ResourceSubsystem->OnResourceChanged.AddDynamic(
    //                this, 
    //                &BAResourceSubsystemUsageExample::OnResourceChanged
    //            );
    //        }
    //    }
    //}
    //
    //// 예시 3: 델리게이트 핸들러
    //UFUNCTION()
    //void OnResourceChanged(EResourceType ResourceType, int32 NewValue)
    //{
    //    // 리소스 변경 시 호출됨
    //    UE_LOG(LogTemp, Log, TEXT("Resource Changed: %d, Value: %d"), 
    //        (int32)ResourceType, NewValue);
    //}
};
