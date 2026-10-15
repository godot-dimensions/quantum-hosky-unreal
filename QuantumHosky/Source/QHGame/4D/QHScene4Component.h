#pragma once

#include "Unreal4DCore/Math/Transform4D.h"

#include "Components/ActorComponent.h"

#include "QHScene4Component.generated.h"

//------------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQHOnTransform4Changed, UQHScene4Component*, InComponent, const FTransform4D&, InNewTransform, const FTransform4D&, InOldTransform);

//______________________________________________________________________________
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class UQHScene4Component
	: public UActorComponent
{
	GENERATED_BODY()

//==============================================================================
public:

	UPROPERTY(BlueprintAssignable, Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	FQHOnTransform4Changed OnTransform4Changed;

//==============================================================================
private:

	UPROPERTY(EditAnywhere, Category = "Scene4")
	FTransform4D Transform4;

	UPROPERTY()
	bool UpgradedTransform4 = false;

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	const FTransform4D& GetTransform4() const
	{
		return Transform4;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void SetTransform4(const FTransform4D& InTransform4);

//==============================================================================
protected:

	//~ UObject BEGIN

	void PostLoad() override;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& InEvent) override;
#endif

	//~ UObject END
};
