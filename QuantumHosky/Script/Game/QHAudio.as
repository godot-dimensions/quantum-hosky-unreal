class UQHSoundEmitterComponent
	: UScene4DComponent
{
// public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 2000.0f;

// private:
	private UAudioComponent AudioComponent = nullptr;

// protected:
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		AudioComponent = Gameplay::SpawnSoundAtLocation(System::LoadAsset_Blocking(Sound), GetWorldLocation(), FRotator::ZeroRotator);
	}

	UFUNCTION(BlueprintOverride)
	protected void EndPlay(const EEndPlayReason InReason)
	{
		AudioComponent.DestroyComponent();
		AudioComponent = nullptr;
	}

	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		const FVector4 Location = GetGlobalPosition4D();

		AudioComponent.SetWorldLocation(QHGame::Position4DTo3D(Location));
		AudioComponent.SetVolumeMultiplier(QH::CalculateSoundVolumeAttenuation4D(Location, InAttenuationDistance3D = Radius, InAttenuationDistanceSlice = (Radius * 0.5f)));
	}
}

namespace QH
{
	float CalculateSoundVolumeAttenuation4D(const FVector4& InLocation, const float InAttenuationDistance3D = 4000.0f, const float InAttenuationDistanceSlice = 1000.0f)
	{
		// NOTE: Direct 4D volume attenuation is technically more `correct`, however qualitatively it can result in very loud sounds if the sound is near you in 4D but very far away from your slice, which feels at odds with 4D mesh projection.
		// Instead, scale volume attenuation by a mixture of 3D distance and slice distance, which results in realistic attenuation when the sound is far away from you both in 3D-space and in slice-space.

		const float ListenerDistance = QHGame::Position4DTo3D(InLocation).Distance(QH::GetCameraTransform().Location);
		const float SliceDistance = QHGame::SliceDistance4D(InLocation);

		const float VolumeMultiplier = Math::Max((1.0f - Math::Square(ListenerDistance / InAttenuationDistance3D)), 0.0f)
			 						 * Math::Max((1.0f - Math::Pow((SliceDistance / InAttenuationDistanceSlice), 3.0f)), 0.0f);

		return Math::Pow(VolumeMultiplier, 3.0f);

		// 4D volume attenuation
		/*
		const float VolumeDistance = InLocation.Distance(QH::GetCameraTransform4D().Translation);

		return Math::Max((1.0f - Math::Square(VolumeDistance / InAttenuationDistance)), 0.0f);
		*/
	}
}
