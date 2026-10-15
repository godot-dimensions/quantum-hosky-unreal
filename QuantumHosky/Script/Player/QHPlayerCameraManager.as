//______________________________________________________________________________
class AQHASPlayerCameraManager
	: AQHPlayerCameraManager
{
	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(float DeltaSeconds)
	{
		AQHASPlayerCharacter PlayerCharacter = QH::GetPlayerCharacter();
		if (PlayerCharacter == nullptr || PlayerCharacter.Scene4Component == nullptr)
		{
			return;
		}

		UCameraComponent CameraComponent = PlayerCharacter.GetComponentByClass(UCameraComponent);

		if (CameraComponent == nullptr)
		{
			return;
		}

		// Offset the player's 4D position by the camera component's local 3D location,
		const FVector CameraRelativeLocation3 = CameraComponent.GetRelativeLocation();
		const FVector4 CameraRelativeLocation4(CameraRelativeLocation3, 0.0f);

		const FQHSlice4 Slice4 = QH::GetActiveSlice4();
		const FTransform4D CameraTransform4(
			Slice4.GetRotation4Matrix(),
			PlayerCharacter.Scene4Component.GetTransform4().Origin + CameraRelativeLocation4);

		// Slice4 rotation is stored only in the 4D camera transform. The ordinary 3D
		// camera rotation is not part of the Slice4 rotation matrix, so we must NOT pass
		// it here as part of the 3D camera transform.
		//
		// UpdateBoundsForCamera() in UMesh4DComponent reconstructs world-space
		// bounds by embedding camera-relative 4D coordinates back into UE world space using
		// the provided 3D transform. For this embedding to be correct, the 3D transform
		// must represent only the world-space translation anchor of the camera, without
		// any additional rotation.
		//
		// If a non-identity 3D rotation is applied here, it introduces a mismatch between
		// the 4D camera frame and the 3D embedding frame, which results in incorrect bounds
		// placement (e.g. drifting or inversion under W-plane rotations).
		//
		// This also matches the GPU slicing path: the 3D camera transform is used only as
		// a compensation frame, and its rotation is expected to be identity in Slice4 mode.
		const FTransform CameraTransform3 = CameraComponent.GetWorldTransform();
		FTransform NewCameraTransform3 = CameraTransform3;
		NewCameraTransform3.SetRotation(FQuat::Identity);

		URenderingServer4D Rendering4System = URenderingServer4D::Get();
		Rendering4System.SetOverrideCameraTransforms(CameraTransform4, NewCameraTransform3);
	}
}

//______________________________________________________________________________
namespace QH
{
	////////////////////////////////////////////////////////////////////////////////
	AQHPlayerCameraManager GetPlayerCameraManager()
	{
		return Cast<AQHPlayerCameraManager>(Gameplay::GetPlayerCameraManager(0));
	}

	////////////////////////////////////////////////////////////////////////////////
	FTransform GetCameraTransform()
	{
		APlayerCameraManager CameraManager = QH::GetPlayerCameraManager();

		return (CameraManager != nullptr)
			 ? FTransform(CameraManager.GetCameraRotation(), CameraManager.GetCameraLocation())
			 : FTransform();
	}

	////////////////////////////////////////////////////////////////////////////////
	FTransform4D GetCameraTransform4D()
	{
		URenderingServer4D RenderingServer = URenderingServer4D::Get();

		return RenderingServer.GetCurrentCameraTransform4D();
	}
}
