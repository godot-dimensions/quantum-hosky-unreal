//______________________________________________________________________________
class AQHASPlayerController
	: APlayerController
{
	default PlayerCameraManagerClass = AQHASPlayerCameraManager;

//==============================================================================
// public:

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHSlice4ControlComponent Slice4ControlComponent = nullptr;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHPlayerItemObserverComponent ItemObserverComponent = nullptr;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHPlayerPlacementComponent PlacementComponent = nullptr;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHPlayerInteractorComponent InteractorComponent = nullptr;
}

//______________________________________________________________________________
namespace QH
{
	////////////////////////////////////////////////////////////////////////////////
	AQHASPlayerController GetPlayerController()
	{
		return Cast<AQHASPlayerController>(Gameplay::GetPlayerController(0));
	}

	////////////////////////////////////////////////////////////////////////////////
	UEnhancedInputComponent GetInputComponent()
	{
		return UEnhancedInputComponent::Get(QH::GetPlayerController());
	}
}
