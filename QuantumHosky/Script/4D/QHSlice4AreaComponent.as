//______________________________________________________________________________
class UQHSlice4AreaComponent
	: UActorComponent
{

//==============================================================================
// public:

	/** If set, will use the specified actor as the area volume for the slice.
	  * If empty, will use the owning actor for the area volume for the slice.
	  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	TSoftObjectPtr<AActor> Slice4AreaActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	bool ResetSlice4OnEnter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	bool ResetSlice4OnExit = false;

//==============================================================================
// private:

	private UQHASSlice4Component Slice4Component = nullptr;

	private AActor SliceAreaActorPtr = nullptr;

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnActorBeginOverlap(AActor InThisActor, AActor InOtherActor)
	{
		if (Slice4Component != nullptr)
		{
			QH::PushActiveSlice4Component(Slice4Component);

			if (ResetSlice4OnEnter)
			{
				Slice4Component.ResetSlice4();
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnActorEndOverlap(AActor InThisActor, AActor InOtherActor)
	{
		if (Slice4Component != nullptr)
		{
			QH::PopActiveSlice4Component(Slice4Component);

			if (ResetSlice4OnExit)
			{
				Slice4Component.ResetSlice4();
			}
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

		SliceAreaActorPtr = !Slice4AreaActor.IsNull()
						  ? System::LoadAsset_Blocking(Slice4AreaActor)
						  : Owner;

		if (SliceAreaActorPtr != nullptr)
		{
			SliceAreaActorPtr.OnActorBeginOverlap.AddUFunction(this, n"OnActorBeginOverlap");
			SliceAreaActorPtr.OnActorEndOverlap.AddUFunction(this, n"OnActorEndOverlap");
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void EndPlay(const EEndPlayReason InReason)
	{
		if (SliceAreaActorPtr != nullptr)
		{
			SliceAreaActorPtr.OnActorBeginOverlap.UnbindObject(this);
			SliceAreaActorPtr.OnActorEndOverlap.UnbindObject(this);
		}
	}

	//~ UActorComponent END
}

//______________________________________________________________________________
class AQHSlice4AreaActor
	: AQHASSlice4Actor
{

//==============================================================================
// public:

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHSlice4AreaComponent Slice4AreaComponent = nullptr;
}

//______________________________________________________________________________
class AQHSlice4AreaBox
	: AQHSlice4AreaActor
{

//==============================================================================
// public:

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent BoxComponent = nullptr;
	default BoxComponent.CollisionProfileName = CollisionProfile::OverlapOnlyPawn;
}
