#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "QHSliceCollision3DSubsystem.generated.h"

class UMesh4DComponent;
class UQHSliceCollision3DComponent;
class URenderingServer4D;

// Owns the transient collision companions for meshes registered in this world.
UCLASS()
class QHGAME_API UQHSliceCollision3DSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& InCollection) override;
	void Deinitialize() override;
	bool DoesSupportWorldType(EWorldType::Type InWorldType) const override;

	UFUNCTION(BlueprintPure, Category = "Quantum Hosky|Physics")
	UQHSliceCollision3DComponent* GetCollisionComponent(UMesh4DComponent* InMesh) const;

private:
	UFUNCTION()
	void OnMeshRegistered(UMesh4DComponent* InMesh);
	UFUNCTION()
	void OnMeshUnregistered(UMesh4DComponent* InMesh);
	void UpdateMeshes(UWorld* InWorld);

	UPROPERTY(Transient)
	TWeakObjectPtr<URenderingServer4D> RenderingServer;

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<UMesh4DComponent>, TObjectPtr<UQHSliceCollision3DComponent>> CollisionComponents;

	TSet<TWeakObjectPtr<UMesh4DComponent>> PendingMeshes;
};
