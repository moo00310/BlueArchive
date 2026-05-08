// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAPartySelectWidget.h"
#include "UI/BAPreviewSlotPanelWidget.h"
#include "UI/BAPreviewSlotInputWidget.h"
#include "UI/BAUser_SDF_DecoWidget.h"
#include "UI/BACharacterPortraitWidget.h"
#include "UI/BAUserWidgetRadio.h"
#include "UI/BASelectPopUpWidget.h"
#include "UI/ViewModel/BAPartyViewModel.h"
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

	// ─── ViewModel 생성 ───
	PartyViewModel = NewObject<UBAPartyViewModel>(this);
	PartyViewModel->Init(GetGameInstance());
	PartyViewModel->OnPartyIdsChanged.AddDynamic(this, &UBAPartySelectWidget::OnPartyIdsChangedHandler);

	// ─── 프리셋 라디오 ───
	if (Radio_PresetSelector)
	{
		Radio_PresetSelector->OnSelectionChanged.AddDynamic(this, &UBAPartySelectWidget::HandlePresetSelectionChanged);
		Radio_PresetSelector->SetSelectedIndex(PartyViewModel->GetCurrentPresetIndex(), false);
	}

	// ─── 팝업 초기화 ───
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

	// ─── 프리뷰 슬롯 셋업 ───
	PreviewSlots.SetNum(4);

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
			Panel->OnSlotDropTarget.AddDynamic(this, &UBAPartySelectWidget::HandlePreviewSlotDropTarget);
			Panel->OnPreviewSlotUnpressed.AddDynamic(this, &UBAPartySelectWidget::HandlePreviewSlotUnpressed);
			Panel->OnPreviewDragStarted.AddDynamic(this, &UBAPartySelectWidget::HandlePreviewSlotDragStarted);
			PreviewSlots[Index].Image = Panel->GetPreviewImage();
		}
		else if (InputWidget)
		{
			InputWidget->SetSlotIndex(Index);
			InputWidget->OnLongPress.AddDynamic(this, &UBAPartySelectWidget::HandlePreviewSlotLongPress);
			InputWidget->OnShortClick.AddDynamic(this, &UBAPartySelectWidget::HandlePreviewSlotShortClick);
			InputWidget->OnPreviewSlotUnpressed.AddDynamic(this, &UBAPartySelectWidget::HandlePreviewSlotUnpressed);
			InputWidget->OnDragStarted.AddDynamic(this, &UBAPartySelectWidget::HandlePreviewSlotDragStarted);
			PreviewSlots[Index].Image = InputWidget->GetPreviewImage();
		}
		else
		{
			PreviewSlots[Index].Image = FallbackImage;
		}
	};
	SetupSlot(0, PreviewSlotPanel_0, PreviewSlotInput_0, IMG_Preview_0);
	SetupSlot(1, PreviewSlotPanel_1, PreviewSlotInput_1, IMG_Preview_1);

	for (int32 i = 0; i < MaxMembersPerParty; ++i)
	{
		if (UBAUser_SDF_DecoWidget* slot = GetPartySlotForIndex(i))
		{
			slot->SetSlotIndex(i);
			slot->OnSlotClicked.AddDynamic(this, &UBAPartySelectWidget::HandleSlotClicked);
		}
	}

	// ViewModel Init 시 이미 LoadPreset(0) 호출됨 → OnPartyIdsChanged가 이미 fired
	// 하지만 NativeConstruct 시점에 바인딩 전이라 수동으로 한 번 더 동기화
	RefreshPartySlots();
	InitPreviewSlot(0);
	InitPreviewSlot(1);
}

// ─── ViewModel 이벤트 ───

void UBAPartySelectWidget::OnPartyIdsChangedHandler()
{
	RefreshPartySlots();
	RefreshPreviewSlot(0);
	RefreshPreviewSlot(1);
}

// ─── 상태 접근자 ───

TArray<FName> UBAPartySelectWidget::GetDisplayPartyIds() const
{
	return PartyViewModel ? PartyViewModel->GetPartyIds() : TArray<FName>();
}

int32 UBAPartySelectWidget::GetCurrentPresetIndex() const
{
	return PartyViewModel ? PartyViewModel->GetCurrentPresetIndex() : 0;
}

// ─── Party API ───

void UBAPartySelectWidget::LoadPartyFromSubsystem()
{
	if (PartyViewModel)
	{
		PartyViewModel->SwitchPreset(PartyViewModel->GetCurrentPresetIndex());
	}
}

void UBAPartySelectWidget::SavePartyToSubsystem()
{
	if (PartyViewModel)
	{
		PartyViewModel->SaveParty();
	}
}

void UBAPartySelectWidget::SwitchPreset(int32 PresetIndex)
{
	if (!PartyViewModel) return;
	if (PresetIndex < 0 || PresetIndex >= MaxPartyPresets) return;

	PartyViewModel->SwitchPreset(PresetIndex);

	// Radio 동기화 (bBroadcast=false 로 무한 루프 방지)
	if (Radio_PresetSelector)
	{
		Radio_PresetSelector->SetSelectedIndex(PartyViewModel->GetCurrentPresetIndex(), false);
	}
}

void UBAPartySelectWidget::SetSlotCharacter(int32 SlotIndex, FName CharacterId)
{
	if (!PartyViewModel) return;
	PartyViewModel->SetSlotCharacter(SlotIndex, CharacterId);

	if (SlotIndex >= 0 && SlotIndex <= 1)
	{
		UE_LOG(LogBAPartyPreview, Log, TEXT("SetSlotCharacter → RefreshPreviewSlot(%d) (CharacterId=%s)"), SlotIndex, *CharacterId.ToString());
		RefreshPreviewSlot(SlotIndex);
	}
}

void UBAPartySelectWidget::SelectCharacterForCurrentSlot(FName CharacterId)
{
	if (SelectedSlotIndex < 0 || SelectedSlotIndex >= MaxMembersPerParty) return;
	SetSlotCharacter(SelectedSlotIndex, CharacterId);
}

void UBAPartySelectWidget::ClearSlot(int32 SlotIndex)
{
	SetSlotCharacter(SlotIndex, NAME_None);
}

void UBAPartySelectWidget::RefreshPartySlots()
{
	if (!PartyViewModel) return;

	const TArray<FName>& Ids = PartyViewModel->GetPartyIds();

	for (int32 i = 0; i < MaxMembersPerParty; ++i)
	{
		if (UBAUser_SDF_DecoWidget* slot = GetPartySlotForIndex(i))
		{
			slot->SetCharacterId(Ids.IsValidIndex(i) ? Ids[i] : NAME_None);
		}
	}

	if (PartyPort_Sup)
	{
		PartyPort_Sup->SetCharacterId(Ids.IsValidIndex(2) ? Ids[2] : NAME_None);
	}
}

// ─── 팝업 ───

void UBAPartySelectWidget::OpenSlotPopup()
{
	if (!CharacterSelectPopup || !PartyViewModel) return;

	if (UBASelectPopUpWidget* PopUp = Cast<UBASelectPopUpWidget>(CharacterSelectPopup))
	{
		PopUp->SetListCharacterIds(PartyViewModel->GetOwnedCharacterIds());
		PopUp->SetCurrentPartyIds(PartyViewModel->GetPartyIds());
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

// ─── Window ───

UUserWidget* UBAPartySelectWidget::ShowWindow(TSubclassOf<UUserWidget> WindowClass)
{
	if (!WindowClass || !WindowLayer) return nullptr;
	CloseWindow();
	UUserWidget* NewWindow = CreateWidget<UUserWidget>(GetOwningPlayer(), WindowClass);
	if (!NewWindow) return nullptr;
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

// ─── 내부 핸들러 ───

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

void UBAPartySelectWidget::HandlePartyConfirmed(TArray<FName> PartyIds)
{
	if (PartyViewModel)
	{
		PartyViewModel->SetAllSlots(PartyIds);
		PartyViewModel->SaveParty();
	}
	UE_LOG(LogBAPartyPreview, Log, TEXT("HandlePartyConfirmed → SetAllSlots + Save"));
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
	if (SlotIndex < 0 || SlotIndex > 1) return;
}

void UBAPartySelectWidget::HandlePreviewSlotUnpressed(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex > 1) return;
	if (ABAPlayerController* PC = GetOwningPlayer<ABAPlayerController>())
	{
		UE_LOG(LogBAPartyPreview, Log, TEXT("[Widget] Unpressed Slot=%d -> Pressed=false"), SlotIndex);
		PC->SetPreviewSlotPressed(SlotIndex, false);
	}
}

void UBAPartySelectWidget::HandlePreviewSlotDragStarted(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex > 1) return;
	if (ABAPlayerController* PC = GetOwningPlayer<ABAPlayerController>())
	{
		UE_LOG(LogBAPartyPreview, Log, TEXT("[Widget] DragStarted Slot=%d -> Pressed=true"), SlotIndex);
		PC->SetPreviewSlotPressed(SlotIndex, true);
	}
}

void UBAPartySelectWidget::HandlePreviewSlotShortClick(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxMembersPerParty) return;
	SelectedSlotIndex = SlotIndex;
	OpenSlotPopup();
}

void UBAPartySelectWidget::HandlePreviewSlotDropTarget(int32 FromSlotIndex, int32 ToSlotIndex)
{
	if (FromSlotIndex < 0 || FromSlotIndex > 1 || ToSlotIndex < 0 || ToSlotIndex > 1 || FromSlotIndex == ToSlotIndex)
		return;

	if (PartyViewModel)
	{
		PartyViewModel->SwapSlots(FromSlotIndex, ToSlotIndex);
		PartyViewModel->SaveParty();
	}
	UE_LOG(LogBAPartyPreview, Log, TEXT("Preview slot swap: %d <-> %d"), FromSlotIndex, ToSlotIndex);
}

// ─── 프리뷰 렌더 (View 책임, ViewModel 불필요) ───

UBAUser_SDF_DecoWidget* UBAPartySelectWidget::GetPartySlotForIndex(int32 Index) const
{
	if (Index == 0 && PreviewSlotPanel_0) return PreviewSlotPanel_0->GetPartySlot();
	if (Index == 1 && PreviewSlotPanel_1) return PreviewSlotPanel_1->GetPartySlot();
	if (Index == 2 && PartySlot_Sup)      return PartySlot_Sup;
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
	Make_RT(S.MaskRT,  FLinearColor(0, 0, 0, 0));

	if (!S.UIMID)
		S.UIMID = UMaterialInstanceDynamic::Create(UI_PreviewMat, this);

	S.UIMID->SetTextureParameterValue(TEXT("ColorTex"), S.ColorRT);
	S.UIMID->SetTextureParameterValue(TEXT("MaskTex"),  S.MaskRT);
	S.Image->SetBrushFromMaterial(S.UIMID);

	const TArray<FName>& Ids = PartyViewModel ? PartyViewModel->GetPartyIds() : TArray<FName>();
	if (ABAPlayerController* PC = Cast<ABAPlayerController>(GetOwningPlayer()))
	{
		if (Ids.IsValidIndex(Index) && !Ids[Index].IsNone())
		{
			PC->ActivatePreview(Ids[Index], Index, S.ColorRT, S.MaskRT);
		}
	}
}

void UBAPartySelectWidget::RefreshPreviewSlot(int32 Index)
{
	UE_LOG(LogBAPartyPreview, Log, TEXT("RefreshPreviewSlot 호출됨 Index=%d"), Index);

	if (Index < 0 || Index >= PreviewSlots.Num())
	{
		UE_LOG(LogBAPartyPreview, Warning, TEXT("RefreshPreviewSlot: 인덱스 범위 밖 (Index=%d)"), Index);
		return;
	}

	FPreviewSlot& S = PreviewSlots[Index];
	if (!S.Image || !UI_PreviewMat)
	{
		UE_LOG(LogBAPartyPreview, Warning, TEXT("RefreshPreviewSlot: Image 또는 UI_PreviewMat 없음 (Index=%d)"), Index);
		return;
	}

	if (!S.ColorRT || !S.MaskRT)
	{
		UE_LOG(LogBAPartyPreview, Log, TEXT("RefreshPreviewSlot: 미초기화 → InitPreviewSlot(%d)"), Index);
		InitPreviewSlot(Index);
		return;
	}

	const TArray<FName>& Ids = PartyViewModel ? PartyViewModel->GetPartyIds() : TArray<FName>();
	FName Id = Ids.IsValidIndex(Index) ? Ids[Index] : NAME_None;

	if (Id.IsNone())
	{
		UE_LOG(LogBAPartyPreview, Log, TEXT("RefreshPreviewSlot: 슬롯 비움 (Index=%d)"), Index);
		if (ABAPlayerController* PC = Cast<ABAPlayerController>(GetOwningPlayer()))
		{
			PC->ClearPreview(Index);
		}
		UKismetRenderingLibrary::ClearRenderTarget2D(this, S.ColorRT, FLinearColor(0.f, 0.f, 0.f, 0.f));
		UKismetRenderingLibrary::ClearRenderTarget2D(this, S.MaskRT,  FLinearColor(0.f, 0.f, 0.f, 0.f));
	}
	else
	{
		UE_LOG(LogBAPartyPr eview, Log, TEXT("RefreshPreviewSlot: ActivatePreview (Index=%d, Id=%s)"), Index, *Id.ToString());
		if (ABAPlayerController* PC = Cast<ABAPlayerController>(GetOwningPlayer()))
		{
			PC->ActivatePreview(Id, Index, S.ColorRT, S.MaskRT);
		}
	}
}
