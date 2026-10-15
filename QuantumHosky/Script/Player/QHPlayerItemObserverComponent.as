//______________________________________________________________________________
class UQHPlayerItemObserverComponent
	: UActorComponent
{

//==============================================================================
// private:

	private TArray<AQHASItemActor> ObservedItems;

//==============================================================================
// protected:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(float DeltaSeconds)
	{
		const FTransform CameraTransform = QH::GetCameraTransform();

		const float ItemSearchDistance = 500.0f;
		const float ItemSearchRadius = 30.0f;

		TArray<EObjectTypeQuery> ItemObjectType;
		ItemObjectType.Add(EObjectTypeQuery::Item);

		// Look for items along the camera view
		TArray<FHitResult> ItemHits;
		System::SphereTraceMultiForObjects(CameraTransform.Location, (CameraTransform.Location + (CameraTransform.Rotation.GetForwardVector() * ItemSearchDistance)), ItemSearchRadius, ItemObjectType, false, TArray<AActor>(), EDrawDebugTrace::None, ItemHits, true);

		// Gather observed items

		TArray<AQHASItemActor> NewObservedItems;

		for (const FHitResult& Hit : ItemHits)
		{
			AQHASItemActor Item = Cast<AQHASItemActor>(Hit.Actor);

			if (QHItem::IsValidItemActor(Item))
			{
				NewObservedItems.Add(Item);
			}
		}

		for (AQHASItemActor NewItem : NewObservedItems)
		{
			const int32 OldItemIndex = ObservedItems.FindIndex(NewItem);

			// Persistently observed item
			if (OldItemIndex != -1)
			{
				ObservedItems.RemoveAtSwap(OldItemIndex);
			}
			// Newly observed item
			else
			{
				NewItem.ShowNameplate(true);
			}
		}

		// No longer observed items
		for (AQHASItemActor OldItem : ObservedItems)
		{
			OldItem.ShowNameplate(false);
		}

		ObservedItems = NewObservedItems;
	}
}
