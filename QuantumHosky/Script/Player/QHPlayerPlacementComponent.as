//______________________________________________________________________________
class UQHPlayerPlacementComponent
	: UQHPlacementComponent
{

//==============================================================================
// public:

	//~ UQHPlacementComponent BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride, meta = (NoSuperCall))
	void GetPlacementQueryParams(AActor& OutInstigator, FVector& OutQueryStart, FVector& OutQueryDirection, float& OutQueryDistance) const
	{
		// Place along the camera view vector

		const FTransform CameraTransform = QH::GetCameraTransform();

		OutInstigator = QH::GetPlayerCharacter();
		OutQueryStart = CameraTransform.Location;
		OutQueryDirection = CameraTransform.Rotation.GetForwardVector();
		OutQueryDistance = QH::DefaultPlacementQueryDistance;
	}

	//~ UQHPlacementComponent END

//==============================================================================
// protected:

	//~ AActor BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(float DeltaSeconds)
	{
		// Debug draw placement
		if (CVar_QHPlacementShow.GetBool())
		{
			// TODO: Add support for 4D placement previews.
			// This would require 4D placement queries, and then slicing a 4D cube at the placement location to then render as the preview.

			FHitResult PlacementHit;
			FVector PlacementLocation;
			if (PlacementQuery(ECollisionChannel::ECC_WorldStatic, PlacementHit, PlacementLocation))
			{
				QH::DebugDrawPlacement(PlacementHit, PlacementLocation);
			}
		}
	}

	//~ AActor END
}

//______________________________________________________________________________
const FConsoleVariable CVar_QHPlacementShow("placement.show", false);
