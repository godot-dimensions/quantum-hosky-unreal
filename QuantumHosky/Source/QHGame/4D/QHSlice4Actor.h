#pragma once

class UQHSlice4Component;
class UQHSlice4MeshManagerComponent;

//------------------------------------------------------------------------------

#include "GameFramework/Actor.h"

#include "QHSlice4Actor.generated.h"

//______________________________________________________________________________
UCLASS(BlueprintType, Placeable)
class AQHSlice4Actor
	: public AActor
{
	GENERATED_BODY()

//==============================================================================
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UQHSlice4Component> Slice4Component = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UQHSlice4MeshManagerComponent> Slice4MeshManagerComponent = nullptr;

//==============================================================================
public:

	AQHSlice4Actor(const FObjectInitializer& Init);
};
