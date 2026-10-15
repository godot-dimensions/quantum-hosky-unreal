//______________________________________________________________________________
UCLASS(Abstract)
class AQHWaterVolume
	: AActor
{

//==============================================================================
// public:

	UPROPERTY(DefaultComponent, RootComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent WaterMeshComponent = nullptr;
	default WaterMeshComponent.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	UPROPERTY(DefaultComponent, Attach = WaterMeshComponent, VisibleAnywhere, BlueprintReadOnly)
	UBoxShape4DComponent WaterCollisionComponent = nullptr;
}
