// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BASelectPopUpWidget.generated.h"

class UBACharacterListWidget;

/**
 * 캐릭터 선택 팝업 (WBP_SelectPopUp용).
 * Panel_List 자리에 BACharacterListWidget을 두고, 선택 시 OnCharacterSelected 브로드캐스트.
 * 확인/취소 버튼은 블루프린트에서 이 위젯의 ClosePopUp 또는 ConfirmSelection 호출로 연결.
 */
UCLASS()
class BLUEARCHIVE_API UBASelectPopUpWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	/** 리스트에 표시할 캐릭터 ID 설정 (보유 캐릭터 등). 설정 후 리스트 갱신 */
	UFUNCTION(BlueprintCallable, Category = "PopUp")
	void SetListCharacterIds(const TArray<FName>& InCharacterIds);

	/** 캐릭터 선택 시 브로드캐스트. 부모(BAPartySelectWidget 등)에서 구독 후 SelectCharacterForCurrentSlot + CloseSlotPopup */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSelected, FName, CharacterId);
	UPROPERTY(BlueprintAssignable, Category = "PopUp")
	FOnCharacterSelected OnCharacterSelected;

	/** 팝업 닫기 (취소). 부모에서 CloseSlotPopup 호출하도록 블루프린트에서 연결 */
	UFUNCTION(BlueprintCallable, Category = "PopUp")
	void ClosePopUp();

	/** 선택 확정 시 호출. 리스트에서 선택한 캐릭터가 있으면 OnCharacterSelected 브로드캐스트 후 닫기는 부모에서 처리 */
	UFUNCTION(BlueprintCallable, Category = "PopUp")
	void ConfirmSelection(FName CharacterId);

protected:
	void NativeConstruct() override;

private:
	void HandleListCharacterSelected(FName CharacterId);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBACharacterListWidget> CharacterList;

	/** 리스트에서 마지막으로 선택된 캐릭터 (확인 버튼 시 사용) */
	UPROPERTY()
	FName PendingSelectedCharacterId = NAME_None;
};
