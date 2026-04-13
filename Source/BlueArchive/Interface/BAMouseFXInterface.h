#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UI/UIStructData.h"
#include "BAMouseFXInterface.generated.h"

class UCanvasPanel;

UINTERFACE(BlueprintType)
class UBAMouseFXInterface : public UInterface
{
    GENERATED_BODY()
};

class IBAMouseFXInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MouseFX")
    void InitializeFX(UCanvasPanel* InCanvas);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MouseFX")
    void UpdateFX(const FMouseFXFrame& Frame);
};