////////////////////////////////////////////////////////////////////////////////
// Copyright © Renascence Games - All Rights Reserved

//______________________________________________________________________________
/** Item component used for testing / ensuring proper dispatch of events. */
UCLASS(meta = (DisplayName = "Test"))
class UQHTestItemComponent
	: UQHItemComponent
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere)
	float ItemTickRate = 10.0f;

//==============================================================================
// private:

	float ItemTickTimer = 0.0f;

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private void LogEvent(const FString& InEvent)
	{
		Log(f"{Owner.GetDef().ItemName} - {InEvent}");
	}

//==============================================================================
// protected:

	//~ UQHItemComponent BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void OnBeginPlay()
	{
		LogEvent("BeginPlay");

		SetTickEnabled(ItemTickRate >= 0.0f);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void OnEndPlay()
	{
		LogEvent("EndPlay");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void OnTick(float InDeltaTime)
	{
		ItemTickTimer += InDeltaTime;

		if (ItemTickTimer >= ItemTickRate)
		{
			LogEvent("Tick");

			ItemTickTimer = 0.0f;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void OnAddedToInventory(UQHInventoryComponent InInventoryComponent, int InInventorySlot)
	{
		LogEvent(f"AddedToInventory - {InInventoryComponent.Owner}[{InInventorySlot}]");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void OnRemovedFromInventory(UQHInventoryComponent InInventoryComponent, int InInventorySlot)
	{
		LogEvent(f"RemovedFromInventory - {InInventoryComponent.Owner}[{InInventorySlot}]");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void OnActorSpawned(AQHItemActor InItemActor)
	{
		LogEvent(f"ActorSpawned - {InItemActor}");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void OnActorDestroyed(AQHItemActor InItemActor)
	{
		LogEvent(f"ActorDestroyed - {InItemActor}");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void OnPickedUp(UQHInventoryComponent InInventoryComponent, int InInventorySlot, AQHItemActor InItemActor)
	{
		LogEvent(f"PickedUp - {InInventoryComponent.Owner}[{InInventorySlot}] - {InItemActor}");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void OnDropped(UQHInventoryComponent InInventoryComponent, int InInventorySlot, AQHItemActor InItemActor)
	{
		LogEvent(f"Dropped - {InInventoryComponent.Owner}[{InInventorySlot}] - {InItemActor}");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	TOptional<FQHItemComponentUsageResult> OnUse(FQHItemUsageInfo InInfo, bool InTest)
	{
		LogEvent(f"Use - Instigator={InInfo.Instigator}");

		return TOptional<FQHItemComponentUsageResult>();
	}

	//~ UQHItemComponent END
}

//______________________________________________________________________________
UCLASS(meta = (DisplayName = "Cooldown"))
class UQHCooldownItemComponent
	: UQHItemComponent
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool StartOnCooldown = false;

//==============================================================================
// private:

	private float UsageTimestamp = -MAX_flt;

//==============================================================================
// protected:

	//~ UQHItemComponent BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void OnBeginPlay()
	{
		if (StartOnCooldown)
		{
			UsageTimestamp = System::GetGameTimeInSeconds();
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected TOptional<FQHItemComponentUsageResult> OnUse(FQHItemUsageInfo InInfo, bool InTest)
	{
		FQHItemComponentUsageResult Result;

		const float GameTime = System::GetGameTimeInSeconds();

		if ((GameTime - UsageTimestamp) >= Cooldown)
		{
			if (!InTest)
			{
				// Put on cooldown
				UsageTimestamp = GameTime;
			}

			Result = FQHItemComponentUsageResult(EQHItemUsageResultType::Success);
		}
		else
		{
			Result = FQHItemComponentUsageResult(EQHItemUsageResultType::Failure, FText::FromString("Item on cooldown."));
		}

		return Result;
	}

	//~ UQHItemComponent END
}

//______________________________________________________________________________
/** Spawns an actor and places it on the grid. */
UCLASS(meta = (DisplayName = "Place Actor"))
class UQHPlaceActorItemComponent
	: UQHItemComponent
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	TSoftClassPtr<AActor> PlaceActorType;

//==============================================================================
// protected:

	//~ UQHItemComponent BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	TOptional<FQHItemComponentUsageResult> OnUse(FQHItemUsageInfo InInfo, bool InTest)
	{
		FQHItemComponentUsageResult Result;

		const TSubclassOf<AActor> PlaceActorClass = System::LoadClassAsset_Blocking(PlaceActorType);

		if (PlaceActorClass.IsValid())
		{
			bool PlacementSuccess = false;

			UQHPlacementComponent InstigatorPlacementComponent = QHGame::FindComponent(InInfo.Instigator, UQHPlacementComponent);

			if (InstigatorPlacementComponent != nullptr)
			{
				// Determine if placement is valid;
				// TODO: Place the actor in 4D directly using a 4D placement query rather than a 3D placement query and then converting to 4D as
				// this performs snapping to the 3D grid rather than the 4D grid which can lead to mis-alignment if placing actors off the W=0 slice.

				FHitResult PlacementHit;
				FVector PlacementLocation;
				PlacementSuccess = InstigatorPlacementComponent.PlacementQuery(ECollisionChannel::ECC_WorldStatic, PlacementHit, PlacementLocation);

				if (PlacementSuccess)
				{
					if (!InTest)
					{
						// Spawn the actor
						AActor PlacedActor = SpawnActor(PlaceActorClass, bDeferredSpawn = true);

						// Find an appropriate slice for the actor

						UQHSlice4Component PlacedActorSliceComponent = nullptr;
						UQHSlice4MemberComponent PlacedActorSliceMemberComponent = UQHSlice4MemberComponent::Get(PlacedActor);

						if (PlacedActorSliceMemberComponent != nullptr)
						{
							UQHSlice4MemberComponent InstigatorSliceMemberComponent = UQHSlice4MemberComponent::Get(InInfo.Instigator);

							if (InstigatorSliceMemberComponent != nullptr)
							{
								// Place the actor in the instigator's slice
								PlacedActorSliceComponent = InstigatorSliceMemberComponent.GetOwnerSlice4();
							}

							if (PlacedActorSliceComponent == nullptr)
							{
								// Place the actor in the default slice
								PlacedActorSliceComponent = QH::GetDefaultSlice4Component();
							}

							PlacedActorSliceMemberComponent.SetOwnerSlice4(PlacedActorSliceComponent);
						}

						USceneComponent RootComponent = PlacedActor.GetRootComponent();

						if (RootComponent != nullptr)
						{
							UScene4DComponent Scene4Component = Cast<UScene4DComponent>(RootComponent);

							if (Scene4Component != nullptr)
							{
								// Place the actor into the slice in 4D
								Scene4Component.SetGlobalPosition4D((PlacedActorSliceComponent != nullptr) ? PlacedActorSliceComponent.GetSlice4().Slice3ToWorld4(PlacementLocation) : FVector4(PlacementLocation, 0.0f));
							}
							else
							{
								// Place the actor in 3D
								RootComponent.SetWorldLocation(PlacementLocation);
							}
						}

						FinishSpawningActor(PlacedActor);
					}

					Result = EQHItemUsageResultType::Success;
				}
			}

			if (!PlacementSuccess)
			{
				Result = FQHItemComponentUsageResult(EQHItemUsageResultType::Failure, FText::FromString("Placement query failed."));
			}
		}
		else
		{
			Error(f"Attempted to place invalid actor type from item {Owner.Def.Name}!");

			Result = FQHItemComponentUsageResult(EQHItemUsageResultType::Invalid, FText::FromString("Invalid place actor type."));
		}

		return Result;
	}

	//~ UQHItemComponent END
}
