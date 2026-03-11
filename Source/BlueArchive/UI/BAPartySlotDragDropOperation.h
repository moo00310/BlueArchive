// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "BAPartySlotDragDropOperation.generated.h"

/** 프리뷰 슬롯 0↔1 드래그 스왑 시 드래그 소스 슬롯 인덱스 전달용 */
UCLASS()
class BLUEARCHIVE_API UBAPartySlotDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PartySlot")
	int32 SourceSlotIndex = -1;
};
