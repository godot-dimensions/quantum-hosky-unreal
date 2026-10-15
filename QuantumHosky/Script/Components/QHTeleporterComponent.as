//______________________________________________________________________________
class UQHTeleporterComponent
	: UActorComponent
{

//==============================================================================
// public:

	/** The actor to teleport to on entering the teleporter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	TSoftObjectPtr<AActor> TeleportTargetActor;

	/** If set, will use the specified actor as the volume for the teleporter.
	  * If empty, will use the owning actor as the volume for the teleporter.
	  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	TSoftObjectPtr<AActor> TeleportVolumeActor = nullptr;

//==============================================================================
// private:

	private AActor TeleportTargetActorPtr = nullptr;
	private AActor TeleportVolumeActorPtr = nullptr;

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnActorBeginOverlap(AActor InThisActor, AActor InOtherActor)
	{
		if (TeleportTargetActorPtr != nullptr)
		{
			InOtherActor.Teleport(TeleportTargetActorPtr.GetActorLocation(), TeleportTargetActorPtr.GetActorRotation());
		}
	}

//==============================================================================
// protected:

	//~ UActorComponent BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		TeleportTargetActorPtr = System::LoadAsset_Blocking(TeleportTargetActor);

		TeleportVolumeActorPtr = !TeleportVolumeActor.IsNull()
							   ? System::LoadAsset_Blocking(TeleportTargetActor)
							   : Owner;

		if (TeleportVolumeActorPtr != nullptr)
		{
			TeleportVolumeActorPtr.OnActorBeginOverlap.AddUFunction(this, n"OnActorBeginOverlap");
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void EndPlay(const EEndPlayReason InReason)
	{
		if (TeleportVolumeActorPtr != nullptr)
		{
			TeleportVolumeActorPtr.OnActorBeginOverlap.UnbindObject(this);
		}
	}

	//~ UActorComponent END
}


//______________________________________________________________________________
class AQHTeleporterActor
	: AActor
{

//==============================================================================
// public:

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHTeleporterComponent TeleporterComponent = nullptr;
}

//______________________________________________________________________________
class AQHTeleporterBox
	: AQHTeleporterActor
{

//==============================================================================
// public:

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent BoxComponent = nullptr;
	default BoxComponent.CollisionProfileName = CollisionProfile::OverlapOnlyPawn;
}
