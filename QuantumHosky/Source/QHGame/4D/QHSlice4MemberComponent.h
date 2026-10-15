#pragma once

class UScene4DComponent;
class UQHSlice4Component;

//------------------------------------------------------------------------------

#include "Components/ActorComponent.h"

#include "QHSlice4MemberComponent.generated.h"

//------------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQHOnSlice4MemberOwnerChanged, UQHSlice4MemberComponent*, InSlice4MemberComponent, UQHSlice4Component*, InNewOwner, UQHSlice4Component*, InOldOwner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQHOnSlice4MemberCrossedSlice, UQHSlice4MemberComponent*, InSlice4MemberComponent, const bool, InEnteredSlice);

//______________________________________________________________________________
/** A 'member' belonging to a slice (see `UQHSlice4Component`).
  *
  * A member can only belong to a single slice at a time.
  */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class UQHSlice4MemberComponent
	: public UActorComponent
{
	GENERATED_BODY()

//==============================================================================
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	TSoftObjectPtr<AActor> OwnerSlice4Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	float Extents4 = 100.0f;

	UPROPERTY(BlueprintAssignable, Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	FQHOnSlice4MemberOwnerChanged OnOwnerSlice4Changed;

	UPROPERTY(BlueprintAssignable, Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	FQHOnSlice4MemberCrossedSlice OnCrossedSlice;

//==============================================================================
private:

	UPROPERTY(Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	TObjectPtr<UQHSlice4Component> OwnerSlice4 = nullptr;

	UPROPERTY(Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	TObjectPtr<UScene4DComponent> Scene4Component = nullptr;

	float Slice4Distance = 0.0f;

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	UQHSlice4Component* GetOwnerSlice4() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void SetOwnerSlice4(UQHSlice4Component* InSlice4Component);

	/** Slice distances > 0 are 'outside' the slice, and slice distances <= 0 are 'inside' the slice. */
	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	float GetDistanceToOwnerSlice4() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	bool IsInsideOwnerSlice4() const;

	/** Returns the percentage that the object is 'inside' its slice. 0 = at the surface of the slice; 1 = at the origin of the slice. */
	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	float GetInsideOwnerSlice4Percent() const;

//==============================================================================
private:

	void UpdateSlice4Distance();

	UFUNCTION()
	void OnSlice4Changed(UQHSlice4Component* InSlice4Component, const FQHSlice4& InNewSlice, const FQHSlice4& InOldSlice);

	UFUNCTION()
	void OnTransform4Changed(UScene4DComponent* InScene4Component);

//==============================================================================
protected:

	//~ UActorComponent BEGIN

	void OnRegister() override;
	void OnUnregister() override;
	void BeginPlay() override;

	//~ UActorComponent END
};
