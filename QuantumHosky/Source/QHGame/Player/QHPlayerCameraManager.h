#pragma once

#include "Camera/PlayerCameraManager.h"

#include "QHPlayerCameraManager.generated.h"

//______________________________________________________________________________
UCLASS(BlueprintType)
class AQHPlayerCameraManager
	: public APlayerCameraManager
{
	GENERATED_BODY()

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	const FMinimalViewInfo& GetViewInfo() const;
};
