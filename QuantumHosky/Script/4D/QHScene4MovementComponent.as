//______________________________________________________________________________
class UQHScene4MovementComponent
	: UActorComponent
{
	default PrimaryComponentTick.bStartWithTickEnabled = true;

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	FVector4 LinearVelocity4 = FVector4(0.0f, 0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	// Radians per second, using Unreal4D plane names and signs.
	FEuler4D AngularVelocity4;

//==============================================================================
// private:

	private UScene4DComponent Scene4Component = nullptr;

//==============================================================================
// protected:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		Scene4Component = QHGame::GetActor4DComponent(Owner);
		if (Scene4Component == nullptr)
		{
			Scene4Component = UScene4DComponent::Get(Owner);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		if (Scene4Component != nullptr)
		{
			Scene4Component.TranslateLocal(LinearVelocity4 * InDeltaTime);
			Scene4Component.RotateRadians(AngularVelocity4 * InDeltaTime);
		}
	}
}
