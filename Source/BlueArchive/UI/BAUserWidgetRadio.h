// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Components/Button.h"
#include "BAUserWidgetRadio.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionChanged, int32, NewIndex);


UCLASS()
class BLUEARCHIVE_API UBAUserWidgetRadio : public UBAUserWidget
{
	GENERATED_BODY()
	
public:
    /** 외부(부모 위젯)가 구독할 이벤트 */
    UPROPERTY(BlueprintAssignable, Category = "Radio")
    FOnSelectionChanged OnSelectionChanged;

    /** 현재 선택된 인덱스 */
    UFUNCTION(BlueprintCallable, Category = "Radio")
    int32 GetSelectedIndex() const { return SelectedIndex; }

    /** 선택 강제 변경(초기화/외부 동기화용) */
    UFUNCTION(BlueprintCallable, Category = "Radio")
    void SetSelectedIndex(int32 NewIndex, bool bBroadcast = true);

protected:
    virtual void NativeOnInitialized() override;

    /** WBP에서 만든 버튼들 이름을 맞춰야 함 */
    UPROPERTY(meta = (BindWidget)) UButton* Button_Party1 = nullptr;
    UPROPERTY(meta = (BindWidget)) UButton* Button_Party2 = nullptr;
    UPROPERTY(meta = (BindWidget)) UButton* Button_Party3 = nullptr;
    UPROPERTY(meta = (BindWidget)) UButton* Button_Party4 = nullptr;

private:
    UPROPERTY() int32 SelectedIndex = -1;

    UFUNCTION() void HandleClickParty1();
    UFUNCTION() void HandleClickParty2();
    UFUNCTION() void HandleClickParty3();
    UFUNCTION() void HandleClickParty4();

    void ApplyVisual(); // 선택 표시 업데이트

};
