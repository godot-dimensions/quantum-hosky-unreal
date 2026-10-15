//______________________________________________________________________________
class UQHInteractionVolumeComponent
	: UBoxShape4DComponent
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InteractionAngle = 45.0f;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	bool WithinInteractionRange(const FVector4& InLocation, const FVector4& InDirection) const
	{
		bool Result = false;

		if (HasWorldPoint(InLocation))
		{
			const float Angle = Math::Acos((GetGlobalPosition4D() - InLocation).GetSafeNormal().Dot(InDirection));

			Result = (Angle <= Math::DegreesToRadians(InteractionAngle));
		}

		return Result;
	}
}

event void FQHOnInteract(UQHInteractableComponent InInteractable, AActor InInstigator);
delegate bool FQHOnCanInteract(const UQHInteractableComponent InInteractable, const AActor InInstigator);

class UQHInteractableComponent
	: UActorComponent
{
// public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText InteractText;

	FQHOnInteract OnInteract;
	FQHOnCanInteract OnCanInteract;

// public:
	void Interact(AActor InInstigator)
	{
		if ((InInstigator != nullptr) && CanInteract(InInstigator))
		{
			OnInteract.Broadcast(this, InInstigator);
		}
	}

	bool CanInteract(AActor InInstigator) const
	{
		return (InInstigator != nullptr)
			&& (!OnCanInteract.IsBound() || OnCanInteract.Execute(this, InInstigator));
	}

// protected:
	// UActorComponent BEGIN
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		UQHInteractionSystem::Get().RegisterInteractable(this);
	}

	UFUNCTION(BlueprintOverride)
	protected void EndPlay(const EEndPlayReason InReason)
	{
		UQHInteractionSystem::Get().UnregisterInteractable(this);
	}
	// UActorComponent BEGIN
}

class UQHInteractionSystem
	: UWorldSubsystem
{
// private:
	private TArray<UQHInteractableComponent> Interactables;

// public:
	void RegisterInteractable(UQHInteractableComponent InInteractable)
	{
		Interactables.AddUnique(InInteractable);
	}

	void UnregisterInteractable(UQHInteractableComponent InInteractable)
	{
		Interactables.RemoveSingleSwap(InInteractable);
	}

	UQHInteractableComponent QueryInteractables(AActor InInstigator, const FVector4& InLocation, const FVector4& InDirection)
	{
		for (UQHInteractableComponent Interactable : Interactables)
		{
			if (Interactable.CanInteract(InInstigator))
			{
				const TArray<UQHInteractionVolumeComponent> InteractionVolumes = Interactable.Owner.GetComponentsByClass(UQHInteractionVolumeComponent);

				for (UQHInteractionVolumeComponent InteractionVolume : InteractionVolumes)
				{
					if (InteractionVolume.WithinInteractionRange(InLocation, InDirection))
					{
						return Interactable;
					}
				}
			}
		}

		return nullptr;
	}
}

namespace QH
{
	UQHInteractableComponent QueryInteractables(AActor InInstigator, const FVector4& InLocation, const FVector4& InDirection)
	{
		return UQHInteractionSystem::Get().QueryInteractables(InInstigator, InLocation, InDirection);
	}
}
