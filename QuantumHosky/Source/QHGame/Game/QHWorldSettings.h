#pragma once

class UQHSlice4Component;

//------------------------------------------------------------------------------

#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

#include "QHWorldSettings.generated.h"

//______________________________________________________________________________
UCLASS()
class AQHWorldSettings
	: public AWorldSettings
{
	GENERATED_BODY()

//==============================================================================
public:

	AQHWorldSettings(const FObjectInitializer& InInit);

//==============================================================================
public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, BlueprintPure = true)
	UQHSlice4Component* GetDefaultSlice4Component() const;

//==============================================================================
private:

#if WITH_EDITOR
	void OnPostWorldInitialization(UWorld* InWorld, const UWorld::InitializationValues InInit) const;
#endif

//==============================================================================
protected:

	//~ UObject BEGIN

	void BeginDestroy() override;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& InEvent) override;
#endif

	//~ UObject END
};
