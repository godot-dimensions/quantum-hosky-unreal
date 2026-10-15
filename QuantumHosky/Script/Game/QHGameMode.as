//______________________________________________________________________________
event void FQHOnGameReady();

//______________________________________________________________________________
class AQHASGameMode
	: AGameMode
{
	default PlayerControllerClass = AQHASPlayerController;
	default DefaultPawnClass = AQHASPlayerCharacter;
	default HUDClass = AQHASHUD;

//==============================================================================
// public:

	UPROPERTY()
	FQHOnGameReady OnGameReady;

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void NotifyGameReady()
	{
		OnGameReady.Broadcast();
	}

//==============================================================================
// protected:

	//~ AActor BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void BeginPlay()
	{
		System::SetTimerForNextTick(this, "NotifyGameReady");
	}

	//~ AActor END
}

////////////////////////////////////////////////////////////////////////////////
namespace QH
{
	////////////////////////////////////////////////////////////////////////////////
	AQHASGameMode GetGameMode()
	{
		return Cast<AQHASGameMode>(Gameplay::GetGameMode());
	}

	////////////////////////////////////////////////////////////////////////////////
	void BindOnGameReady(UObject InObject, const FName InFunction)
	{
		AQHASGameMode GameMode = QH::GetGameMode();

		if (GameMode != nullptr)
		{
			GameMode.OnGameReady.AddUFunction(InObject, InFunction);
		}
	}
}
