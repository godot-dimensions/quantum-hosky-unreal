#pragma once

class UQHSlice4Component;
class UQHSlice4MemberComponent;
class UScene4DComponent;

//------------------------------------------------------------------------------

#include "QHGame/4D/QHSlice4.h"

#include "Components/ActorComponent.h"

#include "QHSlice4Component.generated.h"

//------------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQHOnSlice4Changed, UQHSlice4Component*, InSlice4Component, const FQHSlice4&, InNewSlice, const FQHSlice4&, InOldSlice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQHOnSlice4MembershipChanged, UQHSlice4Component*, InSlice4Component, UQHSlice4MemberComponent*, InSlice4MemberComponent, const bool, InBelongs);

//______________________________________________________________________________
/** Represents a 4D hyperplane embedded in 3D space used for 'slicing' 4D objects such that they can be represented in the 3D world.
  *
  * Objects within the slice are known as 'members' (see `UQHSlice4MemberComponent`). A member can only belong to a single slice at a time.
  */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class QHGAME_API UQHSlice4Component
	: public UActorComponent
{
	GENERATED_BODY()

//==============================================================================
public:

	// TODO: Allow editing of the FQHSlice4 directly via a custom property editor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	FEuler4D Slice4Rotation;

	UPROPERTY(BlueprintAssignable, Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	FQHOnSlice4Changed OnSlice4Changed;

	UPROPERTY(BlueprintAssignable, Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	FQHOnSlice4MembershipChanged OnMembershipChanged;

//==============================================================================
private:

	UPROPERTY()
	FQHSlice4 Slice4;

	UPROPERTY(Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	TArray<UQHSlice4MemberComponent*> Slice4Members;

//==============================================================================
public:

	/** Find the actor's slice membership, inheriting from 4D attachments or the world default when unspecified. */
	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static UQHSlice4Component* FindSlice4ComponentForScene(UScene4DComponent* InComponent);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	const FQHSlice4& GetSlice4() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void SetSlice4(const FQHSlice4& InSlice);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	const TArray<UQHSlice4MemberComponent*>& GetSlice4Members() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	FTransform4D GetSliceTransform4D() const;

//==============================================================================
private:

	void RegisterMember(UQHSlice4MemberComponent* InSlice4MemberComponent);
	void UnregisterMember(UQHSlice4MemberComponent* InSlice4MemberComponent);

	void SetSlice4_Implementation(const FQHSlice4& InSlice);

//==============================================================================
protected:

	//~ UObject BEGIN

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& InEvent) override;
#endif

	//~ UObject END

	//~ UActorComponent BEGIN

	void EndPlay(const EEndPlayReason::Type InReason) override;

	//~ UActorComponent END

//==============================================================================
public:

	friend class UQHSlice4MemberComponent;
};
