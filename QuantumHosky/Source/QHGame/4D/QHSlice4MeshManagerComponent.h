#pragma once

#include "Components/ActorComponent.h"

#include "QHSlice4MeshManagerComponent.generated.h"

//______________________________________________________________________________
/** Base for QH slice projection visibility management. */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class UQHSlice4MeshManagerComponent
	: public UActorComponent
{
	GENERATED_BODY()
};
