UCLASS(Abstract)
class AQHSign
	: AActor
{
	default PrimaryActorTick.bStartWithTickEnabled = false;

// public:
	UPROPERTY(DefaultComponent, RootComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent SignRootComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = SignRootComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHInteractionVolumeComponent InteractionVolumeComponent = nullptr;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHInteractableComponent InteractableComponent = nullptr;
	default InteractableComponent.InteractText = NSLOCTEXT("QHGame", "Interact_Sign", "Read sign");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> SignWidgetType = nullptr;

// private:
	UUserWidget SignWidget = nullptr;

// protected:
	// AActor BEGIN
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		InteractableComponent.OnInteract.AddUFunction(this, n"OnInteract");
	}

	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		// Close sign on any player input
		if ((SignWidget != nullptr) && QH::GetPlayerController().IsInputKeyDown(EKeys::AnyKey))
		{
			SignWidget.RemoveFromParent();
			SignWidget = nullptr;

			SetActorTickEnabled(false);
		}
	}
	// AActor END

// private:
	UFUNCTION()
	private void OnInteract(UQHInteractableComponent InInteractable, AActor InInstigator)
	{
		if (SignWidget == nullptr)
		{
			APlayerController PlayerController = QH::GetPlayerController();

			SignWidget = WidgetBlueprint::CreateWidget(SignWidgetType, PlayerController);

			if (SignWidget != nullptr)
			{
				SignWidget.AddToViewport();

				// Flush player input so that initial sign interaction doesnt immediately close the sign
				EnhancedInput::FlushPlayerInput(PlayerController);

				SetActorTickEnabled(true);
			}
		}
	}
}
