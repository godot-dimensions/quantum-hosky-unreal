/*
 * Temporary test pawn for 4D rendering and slicing experiments.
 *
 * Provides a minimal player-like controller used to test 4D camera movement,
 * input handling, and slicing behavior in editor (plane editor mode).
 *
 * This class is intended for debugging and prototyping purposes only and is
 * not part of the production gameplay code.
 *
 * Note: The implementation was initially generated with AI assistance and may
 * require refactoring or cleanup if extended or reused.
 */

#pragma once

#include "Unreal4DCore/Scene/Components/Camera4DComponent.h"
#include "GameFramework/Pawn.h"
#include "QHPlayer4DPawn_Test.generated.h"

class UScene4DComponent;
class UCamera4DComponent;

UCLASS(BlueprintType)
class QHGAME_API AQHPlayer4DPawn_Test : public APawn
{
	GENERATED_BODY()

public:
	AQHPlayer4DPawn_Test(const FObjectInitializer& Init);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	UScene4DComponent* GetSelf4DComponent() const { return Root4DComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "4D")
	TObjectPtr<UScene4DComponent> Root4DComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "4D")
	TObjectPtr<UCamera4DComponent> Camera4DComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	double MoveSpeed = 1200.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	double VerticalMoveSpeed = 1200.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation 4D")
	double RotateXWDegreesPerSecond = 90.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse Look")
	double MouseSensitivityYaw = 0.42;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse Look")
	double MouseSensitivityPitch = 0.42;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse Look")
	double MinPitchDegrees = -89.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse Look")
	double MaxPitchDegrees = 89.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bAutoMakeCameraCurrent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mouse Look")
	bool bCaptureMouseOnBeginPlay = true;

private:
	void ApplyMovementInput(float DeltaSeconds);
	void Apply4DRotationInput(float DeltaSeconds);
	void ApplyMouseLook(float DeltaSeconds);

private:
	double CurrentPitchDegrees = 0.0;
};
