#include "QHPlayer4DPawn_Test.h"

#include "Unreal4DCore/Scene/Components/Scene4DComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

AQHPlayer4DPawn_Test::AQHPlayer4DPawn_Test(const FObjectInitializer& Init)
	: Super(Init)
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	Root4DComponent = CreateDefaultSubobject<UScene4DComponent>(TEXT("Root4DComponent"));
	SetRootComponent(Root4DComponent);

	Camera4DComponent = CreateDefaultSubobject<UCamera4DComponent>(TEXT("Camera4DComponent"));
	Camera4DComponent->SetupAttachment(RootComponent);
	Camera4DComponent->SetRelativeTransform(FTransform::Identity);
}

void AQHPlayer4DPawn_Test::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoMakeCameraCurrent && Camera4DComponent != nullptr)
	{
		Camera4DComponent->SetCurrent(true);
	}

	if (Root4DComponent != nullptr)
	{
		Root4DComponent->UpdatedTransform4D();
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC != nullptr && bCaptureMouseOnBeginPlay)
	{
		PC->bShowMouseCursor = false;
		PC->bEnableClickEvents = false;
		PC->bEnableMouseOverEvents = false;

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}
}

void AQHPlayer4DPawn_Test::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ApplyMouseLook(DeltaSeconds);
	Apply4DRotationInput(DeltaSeconds);
	ApplyMovementInput(DeltaSeconds);
}

void AQHPlayer4DPawn_Test::ApplyMovementInput(float DeltaSeconds)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC == nullptr || Root4DComponent == nullptr)
	{
		return;
	}

	FVector4 LocalMove = FVector4::Zero();

	// Forward / backward in local 4D space.
	if (PC->IsInputKeyDown(EKeys::W) || PC->IsInputKeyDown(EKeys::Up))
	{
		LocalMove.X += MoveSpeed * DeltaSeconds;
	}
	if (PC->IsInputKeyDown(EKeys::S) || PC->IsInputKeyDown(EKeys::Down))
	{
		LocalMove.X -= MoveSpeed * DeltaSeconds;
	}

	// Strafe left / right in local 4D space.
	if (PC->IsInputKeyDown(EKeys::D))
	{
		LocalMove.Y += MoveSpeed * DeltaSeconds;
	}
	if (PC->IsInputKeyDown(EKeys::A))
	{
		LocalMove.Y -= MoveSpeed * DeltaSeconds;
	}

	// Up / down in local 4D space.
	if (PC->IsInputKeyDown(EKeys::SpaceBar))
	{
		LocalMove.Z += VerticalMoveSpeed * DeltaSeconds;
	}
	if (PC->IsInputKeyDown(EKeys::LeftControl) || PC->IsInputKeyDown(EKeys::C))
	{
		LocalMove.Z -= VerticalMoveSpeed * DeltaSeconds;
	}

	if (!LocalMove.IsNearlyZero())
	{
		Root4DComponent->TranslateLocal(LocalMove);
	}
}


/*
 * TODO:
 * Rotation logic is currently approximate and only intended for testing.
 * It works well enough for debugging, but the correct way to drive rotations
 * through UScene4DComponent still needs to be implemented.
 */
void AQHPlayer4DPawn_Test::ApplyMouseLook(float DeltaSeconds)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC == nullptr || Root4DComponent == nullptr)
	{
		return;
	}

	if (PC->IsInputKeyDown(EKeys::RightMouseButton))
	{
		return;
	}

	double MouseDeltaX = 0.0;
	double MouseDeltaY = 0.0;
	PC->GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

	if (FMath::IsNearlyZero(MouseDeltaX) && FMath::IsNearlyZero(MouseDeltaY))
	{
		return;
	}

	// Horizontal look: rotate in XY plane.
	const double YawDeltaDegrees = MouseDeltaX * MouseSensitivityYaw;

	// Vertical look: rotate in XZ plane, but clamp accumulated pitch.
	const double DesiredPitchDelta = -MouseDeltaY * MouseSensitivityPitch;
	const double NewPitchDegrees = FMath::Clamp(
		CurrentPitchDegrees + DesiredPitchDelta,
		MinPitchDegrees,
		MaxPitchDegrees
	);
	const double ActualPitchDelta = NewPitchDegrees - CurrentPitchDegrees;
	CurrentPitchDegrees = NewPitchDegrees;

	if (!FMath::IsNearlyZero(YawDeltaDegrees) || !FMath::IsNearlyZero(ActualPitchDelta))
	{
		Root4DComponent->RotateLocalDegrees(
			FEuler4D(
				0.0,               // YZ
				ActualPitchDelta,  // ZX
				0.0,			   // XY
				0.0,               // XW
				0.0,               // YW
				0.0                // ZW
			)
		);
		Root4DComponent->RotateLocalDegrees(
			FEuler4D(
				0.0,               // YZ
				0.0,			   // ZX
				YawDeltaDegrees,   // XY
				0.0,               // XW
				0.0,               // YW
				0.0                // ZW
			)
		);
	}
}


/*
 * TODO:
 * Rotation logic is currently approximate and only intended for testing.
 * It works well enough for debugging, but the correct way to drive rotations
 * through UScene4DComponent still needs to be implemented.
 */
void AQHPlayer4DPawn_Test::Apply4DRotationInput(float DeltaSeconds)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC == nullptr || Root4DComponent == nullptr)
	{
		return;
	}

	if (!PC->IsInputKeyDown(EKeys::RightMouseButton))
	{
		return;
	}

	double MouseDeltaX = 0.0;
	double MouseDeltaY = 0.0;
	PC->GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

	if (FMath::IsNearlyZero(MouseDeltaX) && FMath::IsNearlyZero(MouseDeltaY))
	{
		return;
	}

	// Horizontal look: rotate in XY plane.
	const double YawDeltaDegrees = MouseDeltaX * MouseSensitivityYaw;

	// Vertical look: rotate in XZ plane, but clamp accumulated pitch.
	const double DesiredPitchDelta = -MouseDeltaY * MouseSensitivityPitch;
	const double NewPitchDegrees = FMath::Clamp(
		CurrentPitchDegrees + DesiredPitchDelta,
		MinPitchDegrees,
		MaxPitchDegrees
	);
	const double ActualPitchDelta = NewPitchDegrees - CurrentPitchDegrees;
	CurrentPitchDegrees = NewPitchDegrees;

	if (!FMath::IsNearlyZero(YawDeltaDegrees) || !FMath::IsNearlyZero(ActualPitchDelta))
	{
		Root4DComponent->RotateLocalDegrees(
			FEuler4D(
				0.0,               // YZ
				0.0,               // ZX
				0.0,               // XY
				ActualPitchDelta,  // XW
				0.0,               // YW
				0.0                // ZW
			)
		);
		Root4DComponent->RotateLocalDegrees(
			FEuler4D(
				0.0,               // YZ
				0.0,               // ZX
				0.0,               // XY
				0.0,               // XW
				YawDeltaDegrees,   // YW
				0.0				   // ZW
			)
		);
	}
}
