//______________________________________________________________________________
class UQHASSlice4MeshManagerComponent
	: UQHSlice4MeshManagerComponent
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, Category = "QH")
	bool ShowMesh4ProjectionsWhileSlice4Active = true;

	UPROPERTY(EditAnywhere, Category = "QH")
	bool ShowMesh4ProjectionsWhileSlice4Inactive = false;

//==============================================================================
// private:

	private UQHASSlice4Component Slice4Component = nullptr;

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private void UpdateSlice4Active()
	{
		if (Slice4Component != nullptr)
		{
			SetAllSliceMeshProjectionsVisible(Slice4Component.IsSlice4Active() ? ShowMesh4ProjectionsWhileSlice4Active : ShowMesh4ProjectionsWhileSlice4Inactive);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	private void SetAllSliceMeshProjectionsVisible(const bool InVisible)
	{
		if (Slice4Component != nullptr)
		{
			const TArray<UQHSlice4MemberComponent>& Slice4Members = Slice4Component.GetSlice4Members();

			for (UQHSlice4MemberComponent Slice4Member : Slice4Members)
			{
				SetSliceMeshProjectionsVisible(Slice4Member, InVisible);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	private void SetSliceMeshProjectionsVisible(UQHSlice4MemberComponent InSlice4MemberComponent, const bool InVisible)
	{
		check(InSlice4MemberComponent != nullptr);

		for (UMesh4DProjectionComponent Projection : InSlice4MemberComponent.Owner.GetComponentsByClass(UMesh4DProjectionComponent))
		{
			Projection.SetVisibility(InVisible);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnSlice4ActiveChanged(UQHASSlice4Component InSlice4Component, const bool InActive)
	{
		UpdateSlice4Active();
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnSlice4MembershipChanged(UQHSlice4Component InSlice4Component, UQHSlice4MemberComponent InSlice4MemberComponent, bool InBelongs)
	{
		check(Slice4Component == InSlice4Component);

		if (InBelongs)
		{
			SetSliceMeshProjectionsVisible(InSlice4MemberComponent, (Slice4Component.IsSlice4Active() ? ShowMesh4ProjectionsWhileSlice4Active : ShowMesh4ProjectionsWhileSlice4Inactive));
		}
		else
		{
			SetSliceMeshProjectionsVisible(InSlice4MemberComponent, (Slice4Component.IsSlice4Active() ? !ShowMesh4ProjectionsWhileSlice4Active : !ShowMesh4ProjectionsWhileSlice4Inactive));
		}
	}

//==============================================================================
// protected:

	//~ UActorComponent BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		Slice4Component = UQHASSlice4Component::Get(Owner);

		if (Slice4Component != nullptr)
		{
			Slice4Component.OnActiveChanged.AddUFunction(this, n"OnSlice4ActiveChanged");
			Slice4Component.OnMembershipChanged.AddUFunction(this, n"OnSlice4MembershipChanged");

			UpdateSlice4Active();
		}
	}


	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void EndPlay(const EEndPlayReason InReason)
	{
		if (Slice4Component != nullptr)
		{
			Slice4Component.OnActiveChanged.UnbindObject(this);
			Slice4Component.OnMembershipChanged.UnbindObject(this);
		}
	}

	//~ UActorComponent END
}
