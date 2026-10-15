//______________________________________________________________________________
class AQHASSlice4Actor
	: AQHSlice4Actor
{

//==============================================================================
// public:

	UPROPERTY(OverrideComponent = Slice4Component, VisibleAnywhere, BlueprintReadOnly)
	UQHASSlice4Component ASSlice4Component = nullptr;

	UPROPERTY(OverrideComponent = Slice4MeshManagerComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHASSlice4MeshManagerComponent ASSlice4MeshManagerComponent = nullptr;
}
