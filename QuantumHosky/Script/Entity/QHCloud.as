//______________________________________________________________________________
class AQHCloud
	: AActor
{

//==============================================================================
// public:

	UPROPERTY(DefaultComponent, RootComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent Root4DComponent = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FVector4 Velocity4;

//==============================================================================
// private:

	private float TickTimer = 0.0f;
	private float TickFrequency = 0.0f;

//==============================================================================
// protected:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		// Offset initial tick to avoid spiky frames where all clouds tick at once
		TickTimer = Math::RandRange(0.0f, 3.0f);
		TickFrequency = TickTimer;
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		// Manually control ticking rather than leveraging `PrimaryActorTick.TickInterval` to support variable tick frequency to avoid spiky frames
		TickTimer -= InDeltaTime;

		if (TickTimer <= 0.0f)
		{
			const float DeltaTime = (TickFrequency + Math::Abs(TickTimer));
			Root4DComponent.SetGlobalPosition4D(Root4DComponent.GetGlobalPosition4D() + (Velocity4 * DeltaTime));

			TickFrequency = Math::RandRange(0.075f, 0.125f);
			TickTimer = TickFrequency;
		}
	}
}

//______________________________________________________________________________
class UQHCloudSpawnerComponent
	: UActorComponent
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AQHCloud> CloudActorType;

	UPROPERTY(EditAnywhere)
	int32 NumInitialClouds = 10;

	UPROPERTY(EditAnywhere)
	FVector4 CloudSpawnMinPosition;

	UPROPERTY(EditAnywhere)
	FVector4 CloudSpawnMaxPosition;

	UPROPERTY(EditAnywhere)
	FVector2D CloudSpawnScale = FVector2D(3.0f, 8.0f);

	UPROPERTY(EditAnywhere)
	FVector4 Wind;

//==============================================================================
// private:

	private float CloudSpawnTimer = 0.0f;

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private void SpawnCloud()
	{
		const float CloudLifetimeMin = 30.0f;
		const float CloudLifetimeMax = 60.0f;

		UClass CloudActorClass = System::LoadClassAsset_Blocking(CloudActorType);

		if (CloudActorClass != nullptr)
		{
			AQHCloud CloudActor = Cast<AQHCloud>(SpawnActor(CloudActorClass));

			const FVector4 CloudSpawnPosition4(Math::RandRange(CloudSpawnMinPosition.X, CloudSpawnMaxPosition.X),
											   Math::RandRange(CloudSpawnMinPosition.Y, CloudSpawnMaxPosition.Y),
											   Math::RandRange(CloudSpawnMinPosition.Z, CloudSpawnMaxPosition.Z),
											   Math::RandRange(CloudSpawnMinPosition.W, CloudSpawnMaxPosition.W));

			const float CloudScale = Math::RandRange(CloudSpawnScale.X, CloudSpawnScale.Y);

			CloudActor.SetLifeSpan(Math::RandRange(CloudLifetimeMin, CloudLifetimeMax));

			CloudActor.Root4DComponent.SetGlobalPosition4D(CloudSpawnPosition4);
			CloudActor.Root4DComponent.SetGlobalScale4D(FVector4(CloudScale, CloudScale, CloudScale, CloudScale));
			CloudActor.Root4DComponent.SetGlobalRotationDegrees4D(FEuler4D(0.0f, Math::RandRange(0.0f, 360.0f), 0.0f, 0.0f, 0.0f, Math::RandRange(0.0f, 360.0f)));

			CloudActor.Velocity4 = Wind;
		}
	}

//==============================================================================
// protected:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		for (int32 i = 0; i < NumInitialClouds; ++i)
		{
			SpawnCloud();
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		const float CloudSpawnCooldownMin = 10.0f;
		const float CloudSpawnCooldownMax = 20.0f;

		CloudSpawnTimer -= InDeltaTime;

		if (CloudSpawnTimer <= 0.0f)
		{
			CloudSpawnTimer = Math::RandRange(CloudSpawnCooldownMin, CloudSpawnCooldownMax);

			SpawnCloud();
		}
	}
}
