class UQHPlayerInteractorComponent
	: UActorComponent
{
// private:
	private UQHInteractableComponent Interactable = nullptr;

// public:
	UQHInteractableComponent GetInteractable() const
	{
		return Interactable;
	}

// protected:
	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		// TODO: This should be relative to the instigator, however we currently don't have a good way of grabbing their 4D eye-location
		const FTransform4D CameraTransform4D = QH::GetCameraTransform4D();
		const FVector4 CameraForward4D = QHGame::Vector3DTo4D(QH::GetCameraTransform().GetRotation().GetForwardVector());

		// Search for interactables
		UQHInteractableComponent NewInteractable = QH::QueryInteractables(QH::GetPlayerCharacter(), CameraTransform4D.Origin, CameraForward4D);

		if (NewInteractable != Interactable)
		{
			Interactable = NewInteractable;

			// Update HUD if interactable changed
			QH::GetHUD().SetInteractable(Interactable);
		}
	}
}

UCLASS(Abstract)
class UQHInteractWidget
	: UUserWidget
{
// private:
	UPROPERTY(BindWidget)
	private UTextBlock TextWidget = nullptr;

// public:
	void SetInteractable(UQHInteractableComponent InInteractable)
	{
		TextWidget.SetText(InInteractable.InteractText);
	}
}

namespace QH
{
	UQHInteractableComponent GetPlayerInteractable()
	{
		return QH::GetPlayerController().InteractorComponent.GetInteractable();
	}
}
