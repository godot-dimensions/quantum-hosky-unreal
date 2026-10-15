#pragma once

class AQHItemActor;
class UMaterialInterface;

//------------------------------------------------------------------------------

#include "Engine/DeveloperSettings.h"

#include "QHGameSettings.generated.h"

//______________________________________________________________________________
UCLASS(BlueprintType, Config = Game, DefaultConfig)
class UQHGameSettings
	: public UDeveloperSettings
{
	GENERATED_BODY()

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static UQHGameSettings* Get();

//==============================================================================
public:

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftClassPtr<AQHItemActor> DefaultItemActor;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TSoftObjectPtr<UMaterialInterface> DefaultMesh4ProjectionMaterial;
};
