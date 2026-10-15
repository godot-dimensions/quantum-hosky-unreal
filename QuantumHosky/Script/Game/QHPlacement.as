//______________________________________________________________________________
class UQHPlacementComponent
	: UActorComponent
{

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintEvent)
	bool PlacementQuery(const ECollisionChannel InQueryChannel, FHitResult& OutPlacementHit, FVector& OutPlacementLocation)
	{
		AActor QueryInstigator = nullptr;
		FVector QueryStart;
		FVector QueryDirection;
		float QueryDistance = 0.0f;
		GetPlacementQueryParams(QueryInstigator, QueryStart, QueryDirection, QueryDistance);

		return QH::PlacementQuery(QueryInstigator, QueryStart, QueryDirection, InQueryChannel, OutPlacementHit, OutPlacementLocation, QueryDistance);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintEvent)
	void GetPlacementQueryParams(AActor& OutInstigator, FVector& OutQueryStart, FVector& OutQueryDirection, float& OutQueryDistance) const
	{
		OutInstigator = Owner;
		OutQueryStart = Owner.GetActorLocation();
		OutQueryDirection = Owner.GetActorForwardVector();
		OutQueryDistance = QH::DefaultPlacementQueryDistance;
	}
}

//______________________________________________________________________________
namespace QH
{
	const float PlacementSize = 50.0f; // TODO: Replace with block/voxel size.
	const float DefaultPlacementQueryDistance = (PlacementSize * 8);

	////////////////////////////////////////////////////////////////////////////////
	FVector PlacementGridSnap(const FVector& InLocation)
	{
		// Snap placement location to grid;
		// TODO: Add support for 4D grid placement.
		return InLocation.GridSnap(QH::PlacementSize);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(meta = (WorldContext = "InInstigator"))
	bool PlacementQuery(const AActor InInstigator, const FVector& InStart, const FVector& InDirection, const ECollisionChannel InQueryChannel, FHitResult& OutPlacementHit, FVector& OutPlacementLocation, const float InDistance = QH::DefaultPlacementQueryDistance)
	{
		// TODO: Add support for 4D placement queries.
		// A potential implementation is to perform the 3D query, convert it to 4D slice-space, and snap it to the 4D grid.

		bool Result = false;

		if (InInstigator != nullptr)
		{
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(InInstigator);

			Result = System::LineTraceSingleByChannel(OutPlacementHit, InStart, (InStart + (InDirection * InDistance)), InQueryChannel, Params);

			if (Result)
			{
				// Snap placement location to grid;
				// bias in direction of impact normal to break ties on snap boundaries
				OutPlacementLocation = QH::PlacementGridSnap(OutPlacementHit.ImpactPoint + OutPlacementHit.ImpactNormal);
			}
		}

		return Result;
	}

	////////////////////////////////////////////////////////////////////////////////
	void DebugDrawPlacement(const FHitResult& InPlacementHit, const FVector& InPlacementLocation, const float InDuration = 0.0f)
	{
		System::DrawDebugBox(InPlacementLocation, FVector(QH::PlacementSize * 0.5f), FLinearColor::Yellow, Duration = InDuration, Thickness = 0.5f);

		// Placement impact point
		System::DrawDebugPoint(InPlacementHit.ImpactPoint, 10.0f, FLinearColor::Blue, InDuration, EDrawDebugSceneDepthPriorityGroup::Foreground);
		System::DrawDebugString(InPlacementHit.ImpactPoint + (InPlacementHit.ImpactNormal * 10.0f), InPlacementHit.ImpactPoint.ToString(), TextColor = FLinearColor::Blue, Duration = InDuration);

		// Placement location
		System::DrawDebugPoint(InPlacementLocation, 10.0f, FLinearColor::Red, InDuration, EDrawDebugSceneDepthPriorityGroup::Foreground);
		System::DrawDebugString(InPlacementLocation + (FVector::UpVector * 10.0f), InPlacementLocation.ToString(), TextColor = FLinearColor::Red, Duration = InDuration);
	}
}
