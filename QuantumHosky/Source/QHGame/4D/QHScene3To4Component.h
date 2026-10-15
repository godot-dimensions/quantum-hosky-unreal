#pragma once

class UScene4DComponent;
class UQHScene4Component;
class UQHSlice4Component;
class UQHSlice4MemberComponent;
class USceneComponent;

//------------------------------------------------------------------------------

#include "Components/ActorComponent.h"

#include "QHScene3To4Component.generated.h"

//______________________________________________________________________________
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class UQHScene3To4Component
	: public UActorComponent
{
	GENERATED_BODY()

//==============================================================================
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	bool UpdateTransform3OnSlice4Changed = false;

//==============================================================================
private:

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> Scene3Component = nullptr;

	// TODO: Remove after deprecation of `UQHScene4Component`.
	UPROPERTY(Transient)
	TObjectPtr<UQHScene4Component> Scene4Component = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UScene4DComponent> Scene4DComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UQHSlice4MemberComponent> Slice4MemberComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UQHSlice4Component> OwnerSlice4 = nullptr;

	/** Prevents cascading transform updates when updating 3D -> 4D transforms and vice-versa. */
	int32 TransformUpdateLock = 0;

	/** Tracks the previous 3D transform for 3D -> 4D delta transform application. */
	FTransform PreviousTransform3 = FTransform::Identity;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void SetUpdateTransformEnabled(const bool InEnabled);

//==============================================================================
private:

	void UpdateTransform4From3(const bool InAbsolute = true);
	void UpdateTransform3From4();
	void SetOwnerSlice4(UQHSlice4Component* InSlice4Component);

	void OnTransform3Changed(USceneComponent* InComponent, const EUpdateTransformFlags InFlags, const ETeleportType InTeleport);

	UFUNCTION()
	void OnOwnerSlice4Changed(UQHSlice4MemberComponent* InSlice4MemberComponent, UQHSlice4Component* InNewOwner, UQHSlice4Component* InOldOwner);

	UFUNCTION()
	void OnSlice4Changed(UQHSlice4Component* InSlice4Component, const FQHSlice4& InNewSlice, const FQHSlice4& InOldSlice);

//==============================================================================
protected:

	//~ UActorComponent BEGIN

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type InReason) override;

	//~ UActorComponent END
};
