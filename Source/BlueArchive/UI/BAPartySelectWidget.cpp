// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAPartySelectWidget.h"
#include "UI/BAPreviewSlotPanelWidget.h"
#include "UI/BAPreviewSlotInputWidget.h"
#include "UI/BAUser_SDF_DecoWidget.h"
#include "UI/BACharacterPortraitWidget.h"
#include "UI/BAUserWidgetRadio.h"
#include "UI/BASelectPopUpWidget.h"
#include "SubSystem/BACharacterDataSubsystem.h"
#include "Components/PanelWidget.h"
#include "Blueprint/UserWidget.h"
#include "Player/BAPlayerController.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Components/Image.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Widget.h"

DEFINE_LOG_CATEGORY_STATIC(LogBAPartyPreview, Log, All);

void UBAPartySelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Radio_PresetSelector)
	{
		Radio_PresetSelector->OnSelectionChanged.AddDynamic(this, &UBAPartySelectWidget::HandlePresetSelectionChanged);
		Radio_PresetSelector->SetSelectedIndex(CurrentPresetIndex, false);
	}

	if (CharacterSelectPopup)
	{
		CharacterSelectPopup->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (WidgetToDimWhenPopupOpen)
	{
		WidgetToDimWhenPopupOpen->SetVisibility(ESlateVisibility::Collapsed);
		WidgetToDimWhenPopupOpen->SetRenderOpacity(1.f);
	}

	if (UBASelectPopUpWidget* PopUp = Cast<UBASelectPopUpWidget>(CharacterSelectPopup))
	{
		PopUp->OnPartyConfirmed.AddDynamic(this, &UBAPartySelectWidget::HandlePartyConfirmed);
		PopUp->OnCharacterSelected.AddDynamic(this, &UBAPartySelectWidget::HandlePopUpCharacterSelected);
	}

	LoadPartyFromSubsystem();

	PreviewSlots.SetNum(4);
	// 패널(PreviewSlotInput + CharacterInfo 감싼 WBP) 우선, 없으면 입력 위젯만, 없으면 기존 Image
	auto SetupSlot = [this](int32 Index,
		UBAPreviewSlotPanelWidget* Panel,
		UBAPreviewSlotInputWidget* InputWidget,
		UImage* FallbackImage)
	{
		if (Panel)
		{
			Panel->SetSlotIndex(Index);
			Panel->OnLongPress.AddDynamic(this, &UBAPartySelectWidget::HandlePreviewSlotLongPress);
			Panel->OnShortClick.AddDynamic(this, &UBAPartySelectWidget::HandlePreviewSlotShortClick);
			PreviewSlots[Index].Image = Panel->GetPreviewImage();
		}
		else if (InputWidget)
		{
			InputWidget->SetSlotIndex(Index);
			InputWidget->OnLongPress.AddDynamic(this, &UBAPartySelectWidget::HandlePreviewSlotLongPress);
			InputWidget->OnShortClick.AddDynamic(this, &UBAPartySelectWidget::HandlePreviewSlotShortClick);
			PreviewSlots[Index].Image = InputWidget->GetPreviewImage();
		}
		else
		{
			PreviewSlots[Index].Image = FallbackImage;
		}
	};
	SetupSlot(0, PreviewSlotPanel_0, PreviewSlotInput_0, IMG_Preview_0);
	SetupSlot(1, PreviewSlotPanel_1, PreviewSlotInput_1, IMG_Preview_1);

	// 슬롯 클릭 구독: 0/1은 패널 내부 PartySlot, 2는 PartySlot_Sup. 패널 SetSlotIndex(0/1) 이후에 구독.
	for (int32 i = 0; i < MaxMembersPerParty; ++i)
	{
		if (UBAUser_SDF_DecoWidget* slot = GetPartySlotForIndex(i))
		{
			slot->SetSlotIndex(i);
			slot->OnSlotClicked.AddDynamic(this, &UBAPartySelectWidget::HandleSlotClicked);
		}
	}

	InitPreviewSlot(0);
	InitPreviewSlot(1);
}

UBAUser_SDF_DecoWidget* UBAPartySelectWidget::GetPartySlotForIndex(int32 Index) const
{
	if (Index == 0 && PreviewSlotPanel_0)
		return PreviewSlotPanel_0->GetPartySlot();
	if (Index == 1 && PreviewSlotPanel_1)
		return PreviewSlotPanel_1->GetPartySlot();
	if (Index == 2 && PartySlot_Sup)
		return PartySlot_Sup;
	return nullptr;
}

void UBAPartySelectWidget::Make_RT(TObjectPtr<UTextureRenderTarget2D>& OutRT, const FLinearColor& Clear)
{
	if (!OutRT)
	{
		OutRT = NewObject<UTextureRenderTarget2D>(this);
		OutRT->InitCustomFormat(512, 512, PF_B8G8R8A8, false);
		OutRT->ClearColor = Clear;
		OutRT->UpdateResourceImmediate(true);
	}
}

void UBAPartySelectWidget::InitPreviewSlot(int32 Index)
{
	FPreviewSlot& S = PreviewSlots[Index];
	if (!S.Image || !UI_PreviewMat) return;

	Make_RT(S.ColorRT, FLinearColor(0, 0, 0, 1));
	Make_RT(S.MaskRT, FLinearColor(0, 0, 0, 0));

	if (!S.UIMID)
		S.UIMID = UMaterialInstanceDynamic::Create(UI_PreviewMat, this);

	S.UIMID->SetTextureParameterValue(TEXT("ColorTex"), S.ColorRT);
	S.UIMID->SetTextureParameterValue(TEXT("MaskTex"), S.MaskRT);

	S.Image->SetBrushFromMaterial(S.UIMID);

	if (ABAPlayerController* PC = Cast<ABAPlayerController>(GetOwningPlayer()))
	{
		if (DisplayPartyIds.IsValidIndex(Index) && !DisplayPartyIds[Index].IsNone())
		{
			PC->ActivatePreview(DisplayPartyIds[Index], Index, S.ColorRT, S.MaskRT);
		}
	}
}

void UBAPartySelectWidget::RefreshPreviewSlot(int32 Index)
{
	UE_LOG(LogBAPartyPreview, Log, TEXT("RefreshPreviewSlot 호출됨 Index=%d"), Index);

	// 프리뷰 이미지가 있는 슬롯은 0, 1만 해당
	if (Index < 0 || Index >= PreviewSlots.Num())
	{
		UE_LOG(LogBAPartyPreview, Warning, TEXT("RefreshPreviewSlot: 인덱스 범위 밖 → return (Index=%d)"), Index);
		return;
	}

	FPreviewSlot& S = PreviewSlots[Index];
	if (!S.Image || !UI_PreviewMat)
	{
		UE_LOG(LogBAPartyPreview, Warning, TEXT("RefreshPreviewSlot: Image 또는 UI_PreviewMat 없음 → return (Index=%d)"), Index);
		return;
	}

	// 아직 초기화 안 된 슬롯이면 한 번 초기화 후 종료 (InitPreviewSlot이 갱신까지 수행)
	if (!S.ColorRT || !S.MaskRT)
	{
		UE_LOG(LogBAPartyPreview, Log, TEXT("RefreshPreviewSlot: 슬롯 미초기화 → InitPreviewSlot(%d) 호출"), Index);
		InitPreviewSlot(Index);
		return;
	}

	FName Id = DisplayPartyIds.IsValidIndex(Index) ? DisplayPartyIds[Index] : NAME_None;

	if (Id.IsNone())
	{
		UE_LOG(LogBAPartyPreview, Log, TEXT("RefreshPreviewSlot: 슬롯 비움 → 캡처 해제 후 RT 클리어 (Index=%d)"), Index);
		// 슬롯 비움: 먼저 해당 슬롯이 RT에 그리지 않도록 캡처 해제(선택지 B), 그 다음 RT 클리어
		if (ABAPlayerController* PC = Cast<ABAPlayerController>(GetOwningPlayer()))
		{
			PC->ClearPreview(Index);
		}
		UKismetRenderingLibrary::ClearRenderTarget2D(this, S.ColorRT, FLinearColor(0.f, 0.f, 0.f, 0.f));
		UKismetRenderingLibrary::ClearRenderTarget2D(this, S.MaskRT, FLinearColor(0.f, 0.f, 0.f, 0.f));
	}
	else
	{
		UE_LOG(LogBAPartyPreview, Log, TEXT("RefreshPreviewSlot: ActivatePreview 호출 (Index=%d, Id=%s)"), Index, *Id.ToString());
		if (ABAPlayerController* PC = Cast<ABAPlayerController>(GetOwningPlayer()))
		{
			PC->ActivatePreview(Id, Index, S.ColorRT, S.MaskRT);
		}
		else
		{
			UE_LOG(LogBAPartyPreview, Warning, TEXT("RefreshPreviewSlot: GetOwningPlayer()가 BAPlayerController가 아님 (Index=%d)"), Index);
		}
	}
}

void UBAPartySelectWidget::LoadPartyFromSubsystem()
{
	int32 PresetIndex = FMath::Clamp(CurrentPresetIndex, 0, MaxPartyPresets - 1);
	DisplayPartyIds.SetNum(MaxMembersPerParty);
	for (int32 i = 0; i < MaxMembersPerParty; ++i)
	{
		DisplayPartyIds[i] = NAME_None;
	}

	UBACharacterDataSubsystem* Subsystem = GetSubsystem<UBACharacterDataSubsystem>();

	if (Subsystem)
	{
		TArray<FName> Saved = Subsystem->GetPartyPreset(PresetIndex);
		for (int32 i = 0; i < FMath::Min(Saved.Num(), MaxMembersPerParty); ++i)
		{
			DisplayPartyIds[i] = Saved[i];
		}
	}

	RefreshPartySlots();
}

void UBAPartySelectWidget::SavePartyToSubsystem()
{
	TArray<FName> ToSave;
	for (int32 i = 0; i < MaxMembersPerParty && i < DisplayPartyIds.Num(); ++i)
	{
		ToSave.Add(DisplayPartyIds[i]);
	}
	while (ToSave.Num() < MaxMembersPerParty)
	{
		ToSave.Add(NAME_None);
	}

	UBACharacterDataSubsystem* Subsystem = GetSubsystem<UBACharacterDataSubsystem>();
	if (Subsystem)
	{
		int32 PresetIndex = FMath::Clamp(CurrentPresetIndex, 0, MaxPartyPresets - 1);
		Subsystem->SetPartyPreset(PresetIndex, ToSave);
	}
}

void UBAPartySelectWidget::SwitchPreset(int32 PresetIndex)
{
	if (PresetIndex < 0 || PresetIndex >= MaxPartyPresets) return;
	CurrentPresetIndex = PresetIndex;

	LoadPartyFromSubsystem();
	UE_LOG(LogBAPartyPreview, Log, TEXT("SwitchPreset(PresetIndex=%d) → RefreshPreviewSlot(0), (1) 호출"), PresetIndex);
	RefreshPreviewSlot(0);
	RefreshPreviewSlot(1);

	// 라디오 위젯에도 동기화 (bBroadcast=false로 무한 루프 방지)
	if (Radio_PresetSelector)
	{
		Radio_PresetSelector->SetSelectedIndex(CurrentPresetIndex, false);
	}
}

void UBAPartySelectWidget::HandlePresetSelectionChanged(int32 NewIndex)
{
	SwitchPreset(NewIndex);
}

void UBAPartySelectWidget::HandleSlotClicked(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxMembersPerParty) return;

	SelectedSlotIndex = SlotIndex;
	OpenSlotPopup();
}

void UBAPartySelectWidget::SelectCharacterForCurrentSlot(FName CharacterId)
{
	if (SelectedSlotIndex < 0 || SelectedSlotIndex >= MaxMembersPerParty) return;
	SetSlotCharacter(SelectedSlotIndex, CharacterId);
}

void UBAPartySelectWidget::SetSlotCharacter(int32 SlotIndex, FName CharacterId)
{
	if (SlotIndex < 0 || SlotIndex >= MaxMembersPerParty) return;

	while (DisplayPartyIds.Num() < MaxMembersPerParty)
	{
		DisplayPartyIds.Add(NAME_None);
	}
	DisplayPartyIds[SlotIndex] = CharacterId;
	RefreshPartySlots();
	if (SlotIndex >= 0 && SlotIndex <= 1)
	{
		UE_LOG(LogBAPartyPreview, Log, TEXT("SetSlotCharacter → RefreshPreviewSlot(%d) 호출 (CharacterId=%s)"), SlotIndex, *CharacterId.ToString());
		RefreshPreviewSlot(SlotIndex);
	}
}

void UBAPartySelectWidget::RefreshPartySlots()
{
	for (int32 i = 0; i < MaxMembersPerParty; ++i)
	{
		if (UBAUser_SDF_DecoWidget* slot = GetPartySlotForIndex(i))
		{
			FName Id = (i < DisplayPartyIds.Num()) ? DisplayPartyIds[i] : NAME_None;
			slot->SetCharacterId(Id);
		}
	}

	if (PartyPort_Sup)
	{
		FName IdSlot2 = (DisplayPartyIds.Num() > 2) ? DisplayPartyIds[2] : NAME_None;
		PartyPort_Sup->SetCharacterId(IdSlot2);
	}
}

void UBAPartySelectWidget::ClearSlot(int32 SlotIndex)
{
	SetSlotCharacter(SlotIndex, NAME_None);
}

UUserWidget* UBAPartySelectWidget::ShowWindow(TSubclassOf<UUserWidget> WindowClass)
{
	if (!WindowClass || !WindowLayer)
	{
		return nullptr;
	}

	CloseWindow();

	UUserWidget* NewWindow = CreateWidget<UUserWidget>(GetOwningPlayer(), WindowClass);
	if (!NewWindow)
	{
		return nullptr;
	}

	CurrentWindow = NewWindow;
	WindowLayer->AddChild(CurrentWindow);
	return CurrentWindow;
}

void UBAPartySelectWidget::CloseWindow()
{
	if (CurrentWindow && WindowLayer)
	{
		WindowLayer->RemoveChild(CurrentWindow);
		CurrentWindow = nullptr;
	}
}

void UBAPartySelectWidget::OpenSlotPopup()
{
	if (!CharacterSelectPopup) return;

	if (UBASelectPopUpWidget* PopUp = Cast<UBASelectPopUpWidget>(CharacterSelectPopup))
	{
		UBACharacterDataSubsystem* Sub = GetSubsystem<UBACharacterDataSubsystem>();
		if (Sub)
		{
			PopUp->SetListCharacterIds(Sub->GetAllOwnedCharacterIds());
		}
		// 현재 파티를 팝업에 전달해 상단 슬롯에 표시, 리스트 클릭 시 토글(추가/제거) 가능
		PopUp->SetCurrentPartyIds(DisplayPartyIds);
	}

	CharacterSelectPopup->SetVisibility(ESlateVisibility::Visible);
	bSlotPopupOpen = true;

	if (WidgetToDimWhenPopupOpen)
	{
		WidgetToDimWhenPopupOpen->SetRenderOpacity(0.5f);
		WidgetToDimWhenPopupOpen->SetVisibility(ESlateVisibility::Visible);
	}
}

void UBAPartySelectWidget::CloseSlotPopup()
{
	if (!CharacterSelectPopup) return;
	CharacterSelectPopup->SetVisibility(ESlateVisibility::Collapsed);
	bSlotPopupOpen = false;

	if (WidgetToDimWhenPopupOpen)
	{
		WidgetToDimWhenPopupOpen->SetRenderOpacity(0.f);
		WidgetToDimWhenPopupOpen->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UBAPartySelectWidget::HandlePopUpCharacterSelected(FName CharacterId)
{
	if (CharacterId == NAME_None)
	{
		CloseSlotPopup();
	}
}

void UBAPartySelectWidget::HandlePreviewSlotLongPress(int32 SlotIndex)
{
	UE_LOG(LogBAPartyPreview, Log, TEXT("Preview slot long press: %d (애니 변경·드래그는 다음 단계에서)"), SlotIndex);
	// TODO: 해당 슬롯 프리뷰 캐릭터 애니메이션 변경 + 드래그 가능 상태
}

void UBAPartySelectWidget::HandlePreviewSlotShortClick(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxMembersPerParty) return;
	SelectedSlotIndex = SlotIndex;
	OpenSlotPopup();
}

void UBAPartySelectWidget::HandlePartyConfirmed(TArray<FName> PartyIds)
{
	DisplayPartyIds.Empty();
	for (int32 i = 0; i < FMath::Min(PartyIds.Num(), MaxMembersPerParty); ++i)
	{
		DisplayPartyIds.Add(PartyIds[i]);
	}
	while (DisplayPartyIds.Num() < MaxMembersPerParty)
	{
		DisplayPartyIds.Add(NAME_None);
	}
	RefreshPartySlots();
	SavePartyToSubsystem();
	UE_LOG(LogBAPartyPreview, Log, TEXT("HandlePartyConfirmed → RefreshPreviewSlot(0), (1) 호출"));
	RefreshPreviewSlot(0);
	RefreshPreviewSlot(1);
	// CloseSlotPopup(); 
}
