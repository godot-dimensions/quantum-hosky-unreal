#pragma once

class UQHScene3To4Component;
class UQHScene4Component;
class UQHSlice4MemberComponent;

//------------------------------------------------------------------------------

#include "GameFramework/Character.h"

#include "QHCharacter.generated.h"

//______________________________________________________________________________
UCLASS(BlueprintType, Blueprintable, Abstract)
class AQHCharacter
	: public ACharacter
{
	GENERATED_BODY()

//==============================================================================
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UQHScene4Component> Scene4Component = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UQHSlice4MemberComponent> Slice4MemberComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UQHScene3To4Component> Scene3To4Component = nullptr;

//==============================================================================
public:

	AQHCharacter(const FObjectInitializer& Init);
};
