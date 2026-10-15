#pragma once

#include "GameFramework/CharacterMovementComponent.h"

#include "QHCharacterMovementComponent.generated.h"

//______________________________________________________________________________
UENUM(BlueprintType)
enum class EQHCharacterMovementMode
	: uint8
{
	None,
	Climbing,
	Mantling
};

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct FQHCharacterMovementClimbQuery
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> QueryChannel = ECC_WorldStatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector QueryDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QueryDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QueryDimension = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 QuerySamples = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DebugDrawDuration = -1.0f;
};

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct FQHCharacterMovementClimbInfo
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ClimbSurfaceLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ClimbSurfaceNormal = FVector::ZeroVector;
};

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct FQHCharacterMovementMantleQuery
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> QueryChannel = ECC_WorldStatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QueryDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QueryRadius = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QueryHeight = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DebugDrawDuration = -1.0f;
};

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct FQHCharacterMovementMantleInfo
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MantleTargetLocation = FVector::ZeroVector;
};

//______________________________________________________________________________
UCLASS(BlueprintType)
class QHGAME_API UQHCharacterMovementComponent
	: public UCharacterMovementComponent
{
	GENERATED_BODY()

//==============================================================================
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
	float ClimbAcceleration = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
	float ClimbDeceleration = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
	float ClimbMaxSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle")
	float MantleAcceleration = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle")
	float MantleMaxSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle")
	float MantleDismountSpeed = 1000.0f;

//==============================================================================
private:

	UPROPERTY(Transient)
	FQHCharacterMovementClimbQuery ClimbQuery;

	UPROPERTY(Transient)
	FQHCharacterMovementClimbInfo ClimbInfo;

	UPROPERTY(Transient)
	FQHCharacterMovementMantleInfo MantleInfo;

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	bool QueryClimb(FQHCharacterMovementClimbInfo& OutInfo, const FQHCharacterMovementClimbQuery& InQuery) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void StartClimbing(const FQHCharacterMovementClimbQuery& InQuery, const FQHCharacterMovementClimbInfo& InInfo);

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void StopClimbing();

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	bool IsClimbing() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	FQHCharacterMovementClimbInfo GetClimbInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	bool QueryMantle(FQHCharacterMovementMantleInfo& OutInfo, const FQHCharacterMovementMantleQuery& InQuery) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void StartMantling(const FQHCharacterMovementMantleInfo& InInfo);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	bool IsMantling() const;

//==============================================================================
private:

	void PhysClimbing(const float InDeltaTime, const int32 InIterations);
	void PhysMantling(const float InDeltaTime, const int32 InIterations);

//==============================================================================
protected:

	//~ UActorComponent BEGIN

	void TickComponent(const float InDeltaTime, const ELevelTick InTickType, FActorComponentTickFunction *InTickFunction) override;

	//~ UActorComponent END

	//~ UCharacterMovementComponent BEGIN

	float GetMaxSpeed() const override;
	float GetMaxAcceleration() const override;
	float GetMaxBrakingDeceleration() const override;
	void PhysCustom(const float InDeltaTime, const int32 InIterations) override;

	//~ UCharacterMovementComponent END
};
