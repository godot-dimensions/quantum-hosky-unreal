
UCLASS(meta = (PrioritizeCategories = "QH"))
class UQHSliceViewComponent
	: UScene4DComponent
{
// public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	FVector4 SliceOrigin4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	FVector SliceOrigin3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	FEuler4D SliceRotation;

	float BlendTimer = 0.0f;
	float BlendDuration = 0.0f;
	AActor Instigator = nullptr;

// protected:
	// UActorComponent BEGIN
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		UQHInteractableComponent InteractableComponent = UQHInteractableComponent::Get(Owner);

		if (InteractableComponent != nullptr)
		{
			InteractableComponent.OnInteract.AddUFunction(this, n"OnInteract");
		}
	}

	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		if (Instigator != nullptr)
		{
			BlendDuration += InDeltaTime;
			const float BlendRatio = Math::Min(Math4D::SafeDivide(BlendDuration, BlendTimer, 1.0f), 1.0f);

			UQHScene4Component InstigatorScene4 = UQHScene4Component::Get(Instigator);

			if (InstigatorScene4 != nullptr)
			{
				const FTransform4D TargetTransform = GetGlobalTransform4D();
				const FTransform4D InstigatorTransform = InstigatorScene4.GetTransform4();
				const FTransform4D NewTransform = InstigatorTransform.Lerp(TargetTransform, BlendRatio);

				InstigatorScene4.SetTransform4(NewTransform);

				Instigator.SetActorLocation(QH::GetActiveSlice4().World4ToSlice3(NewTransform.Origin));
			}

			APawn Pawn = Cast<APawn>(Instigator);

			if (Pawn != nullptr)
			{
				APlayerController PlayerController = Cast<APlayerController>(Pawn.GetController());

				if (PlayerController != nullptr)
				{
					// We align you along the X of the slice view component in SLICE-space;
					// we set the slice rotation to the owner rotation on interact and so the forward vector in slice-space is the local forward of the slice view component
					const FVector4 TargetForward = GetLocalRotationRadians4D().RotatePoint(FVector4::Forward);
					FRotator ControlRotation = PlayerController.GetControlRotation();
					const float TargetYaw = Math::RadiansToDegrees(Math::Acos(TargetForward.X));
					//const float TargetYaw = 0.0f;
					ControlRotation.Yaw = ControlRotation.Yaw + (FRotator::NormalizeAxis(TargetYaw - ControlRotation.Yaw) * BlendRatio);
					//ControlRotation.Yaw = TargetYaw;

					Print(f"{TargetYaw}");

					PlayerController.SetControlRotation(ControlRotation);
				}
			}

			if (BlendDuration >= BlendTimer)
			{
				UQHScene3To4Component Scene3To4 = UQHScene3To4Component::Get(Instigator);
			 	Scene3To4.SetUpdateTransformEnabled(true);
				//Scene3To4.UpdateTransform3OnSlice4Changed = false;

				Instigator = nullptr;
			}
		}
	}
	// UActorComponent END

// private:
	UFUNCTION(DisplayName = "Set from slice", Category = "QH", meta = (CallInEditor = "true"))
	private void Editor_SetFromCurrentSlice()
	{
		URenderingServer4D Rendering4System = URenderingServer4D::Get();

		if (Rendering4System != nullptr)
		{
			const FTransform4D SliceTransform4 = Rendering4System.GetCurrentCameraTransform4D();
			const FTransform SliceTransform3 = Rendering4System.GetCurrentCameraTransform3D();

			SliceOrigin4 = SliceTransform4.Origin;
			SliceOrigin3 = SliceTransform3.Location;
			SliceRotation = SliceTransform4.GetRotationDegrees();
			// SliceRotation.XY = 0.0f;
			// SliceRotation.XW = 0.0f;
			// SliceRotation.WY = 0.0f;
		}
	}

	UFUNCTION(DisplayName = "View slice", Category = "QH", meta = (CallInEditor = "true"))
	private void Editor_SetCurrentSliceTo()
	{
		URenderingServer4D Rendering4System = URenderingServer4D::Get();

		if (Rendering4System != nullptr)
		{
			FTransform4D SliceTransform4;
			SliceTransform4 = SliceTransform4.SetRotationDegrees(SliceRotation);
			SliceTransform4 = SliceTransform4.SetOrigin(SliceOrigin4);

			FTransform SliceTransform3;
			SliceTransform3.Location = SliceOrigin3;

			//Rendering4System.SetOverrideCameraTransforms(SliceTransform4, SliceTransform3);

#if EDITOR
			Unreal4DEditor::SetEditorCameraFullTransform(SliceTransform4, SliceTransform3);
#endif
		}
	}

	UFUNCTION()
	private void OnInteract(UQHInteractableComponent InInteractable, AActor InInstigator)
	{
		UQHASSlice4Component ActiveSlice = QH::GetActiveSlice4Component();

		if (ActiveSlice != nullptr)
		{
			UQHScene3To4Component Scene3To4 = UQHScene3To4Component::Get(InInstigator);
			Scene3To4.SetUpdateTransformEnabled(false);
			//Scene3To4.UpdateTransform3OnSlice4Changed = true;

			FQHSlice4State SliceState;

			// This works when storing the slice
			//SliceState.Origin4 = SliceOrigin4;
			//SliceState.Origin3 = SliceOrigin3;
			//SliceState.Rotation4 = FEuler4D(SliceRotation.DegToRad());

			const FTransform4D OwnerTransform = QHGame::GetActorLocalTransform4D(Owner);
			SliceState.Origin4 = OwnerTransform.GetOrigin();
			SliceState.Origin3 = QHGame::Position4DTo3D(SliceState.Origin4);
			SliceState.Rotation4 = OwnerTransform.GetRotation();

			// SliceState.Origin4 = QHGame::GetActorTransform4(InInstigator).Origin;
			// SliceState.Origin3 = InInstigator.GetActorLocation();

			const float SliceBlendDuration = QH::DefaultSlice4BlendDuration;
			//const float SliceBlendDuration = 0.0f;

			ActiveSlice.SetSlice4State(SliceState, SliceBlendDuration);

			BlendTimer = SliceBlendDuration;
			BlendDuration = 0.0f;
			Instigator = InInstigator;
		}
	}
}

UCLASS(Abstract, meta = (PrioritizeCategories = "QH"))
class AQHSliceView
	: AActor
{
// public:
	UPROPERTY(DefaultComponent, RootComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent Root4DComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = Root4DComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHSliceViewComponent SliceViewComponent = nullptr;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHInteractableComponent InteractableComponent = nullptr;
	default InteractableComponent.InteractText = NSLOCTEXT("QHGame", "Interact_SliceView", "Align slice");

	UPROPERTY(DefaultComponent, Attach = Root4DComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHInteractionVolumeComponent InteractionVolumeComponent = nullptr;
}
