#pragma once

#include "Components/PrimitiveComponent.h"
#include "Unreal4DCore/Math/Transform4D.h"
#include "QHSliceCollision3DComponent.generated.h"

class UMesh4DComponent;
class UTetraMesh4D;

// Transient Chaos representation of a mesh's current 4D slice.
UCLASS(Transient)
class QHGAME_API UQHSliceCollision3DComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UQHSliceCollision3DComponent(const FObjectInitializer& InInit);
	void SetSourceMeshComponent(UMesh4DComponent* InMesh);
	void UpdateCollisionForSlice(const FTransform4D& InSliceTransform4D, const FTransform& InSliceTransform3D);
	void ClearCollisionData();
	UBodySetup* GetBodySetup() override;
	FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	UFUNCTION(BlueprintPure, Category = "Quantum Hosky|Physics")
	UMesh4DComponent* GetSourceMeshComponent() const;

protected:
	bool ShouldCreatePhysicsState() const override;
	void OnUnregister() override;

private:
	void UnbindCollisionMesh();
	void OnCollisionMeshDataChanged();
	void RebuildCollisionData(UTetraMesh4D* InMesh, const FTransform4D& InMeshToSliceTransform4D, const FTransform& InSliceToConvexTransform3D);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMesh4DComponent> SourceMeshComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UTetraMesh4D> BoundCollisionMesh;

	UPROPERTY(Transient)
	TObjectPtr<UBodySetup> BodySetup = nullptr;

	FDelegateHandle CollisionMeshDataChangedHandle;
	FBoxSphereBounds LocalCollisionBounds = FBoxSphereBounds(FVector::ZeroVector, FVector::ZeroVector, 0.0);
	FTransform4D PreviousMeshToConvexTransform4D;
	bool bCollisionDirty = true;
};
