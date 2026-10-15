#include "QHSliceCollision3DComponent.h"

#include "QHPhysics.h"
#include "QHGame/Utility/QHProfiling.h"
#include "Unreal4DCore/Model/Mesh/Tetra/TetraMesh4D.h"
#include "Unreal4DCore/Scene/Components/Mesh4DComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"

UQHSliceCollision3DComponent::UQHSliceCollision3DComponent(const FObjectInitializer& InInit)
	: Super(InInit)
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCanEverAffectNavigation(false);
	BodyInstance.bAutoWeld = false;
}

void UQHSliceCollision3DComponent::SetSourceMeshComponent(UMesh4DComponent* InMesh)
{
	if (SourceMeshComponent != InMesh)
	{
		UnbindCollisionMesh();
		ClearCollisionData();
		SourceMeshComponent = InMesh;
	}
}

UMesh4DComponent* UQHSliceCollision3DComponent::GetSourceMeshComponent() const
{
	return SourceMeshComponent.Get();
}

UBodySetup* UQHSliceCollision3DComponent::GetBodySetup()
{
	return BodySetup;
}

bool UQHSliceCollision3DComponent::ShouldCreatePhysicsState() const
{
	return BodySetup && !BodySetup->AggGeom.ConvexElems.IsEmpty() && Super::ShouldCreatePhysicsState();
}

FBoxSphereBounds UQHSliceCollision3DComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return LocalCollisionBounds.TransformBy(LocalToWorld);
}

void UQHSliceCollision3DComponent::OnUnregister()
{
	Super::OnUnregister();
	UnbindCollisionMesh();
	bCollisionDirty = true;
}

void UQHSliceCollision3DComponent::UnbindCollisionMesh()
{
	if (UTetraMesh4D* mesh = BoundCollisionMesh.Get())
	{
		mesh->OnMesh4DCacheCleared.Remove(CollisionMeshDataChangedHandle);
	}
	BoundCollisionMesh.Reset();
	CollisionMeshDataChangedHandle.Reset();
}

void UQHSliceCollision3DComponent::OnCollisionMeshDataChanged()
{
	bCollisionDirty = true;
}

void UQHSliceCollision3DComponent::ClearCollisionData()
{
	DestroyPhysicsState();
	if (BodySetup && !BodySetup->AggGeom.ConvexElems.IsEmpty())
	{
		BodySetup->RemoveSimpleCollision();
	}
	LocalCollisionBounds = FBoxSphereBounds(FVector::ZeroVector, FVector::ZeroVector, 0.0);
	UpdateBounds();
	bCollisionDirty = true;
}

void UQHSliceCollision3DComponent::UpdateCollisionForSlice(const FTransform4D& InSliceTransform4D, const FTransform& InSliceTransform3D)
{
	check(IsInGameThread());
	UMesh4DComponent* source = SourceMeshComponent.Get();
	if (!source || !source->IsRegistered())
	{
		ClearCollisionData();
		return;
	}

	// Retain the source's authored 3D collision settings while the companion owns the body.
	if (Mobility != source->Mobility)
	{
		SetMobility(source->Mobility);
	}
	if (GetCollisionObjectType() != source->GetCollisionObjectType())
	{
		SetCollisionObjectType(source->GetCollisionObjectType());
	}
	if (GetCollisionResponseToChannels() != source->GetCollisionResponseToChannels())
	{
		SetCollisionResponseToChannels(source->GetCollisionResponseToChannels());
	}
	SetGenerateOverlapEvents(source->GetGenerateOverlapEvents());
	if (GetCollisionEnabled() != source->GetCollisionEnabled())
	{
		SetCollisionEnabled(source->GetCollisionEnabled());
	}
	if (BodyInstance.GetPhysMaterialOverride() != source->BodyInstance.GetPhysMaterialOverride())
	{
		SetPhysMaterialOverride(source->BodyInstance.GetPhysMaterialOverride());
	}

	UTetraMesh4D* mesh = source->EnableCollision
		? Cast<UTetraMesh4D>(source->CollisionMesh4D ? source->CollisionMesh4D : source->Mesh4D)
		: nullptr;
	if (mesh != BoundCollisionMesh.Get())
	{
		UnbindCollisionMesh();
		if (mesh)
		{
			BoundCollisionMesh = mesh;
			CollisionMeshDataChangedHandle = mesh->OnMesh4DCacheCleared.AddUObject(this, &UQHSliceCollision3DComponent::OnCollisionMeshDataChanged);
		}
		bCollisionDirty = true;
	}
	if (!mesh || GetCollisionEnabled() == ECollisionEnabled::NoCollision || !mesh->IsMeshDataValid())
	{
		if (IsPhysicsStateCreated() || (BodySetup && !BodySetup->AggGeom.ConvexElems.IsEmpty()))
		{
			ClearCollisionData();
		}
		return;
	}

	const FTransform4D meshToSlice = InSliceTransform4D.Inverse().Compose(source->GetGlobalTransform4D());
	const FTransform sliceToConvex = InSliceTransform3D * GetComponentTransform().Inverse();
	// XYZ camera motion is compensated by the 3D embedding frame. Compare the combined mapping
	// (including its W row for the slice plane), rather than rebuilding for either frame separately.
	const FTransform4D meshToConvex = FTransform4D::From3D(sliceToConvex).Compose(meshToSlice);
	if (!bCollisionDirty && meshToConvex.IsEqualApprox(PreviousMeshToConvexTransform4D))
	{
		return;
	}

	RebuildCollisionData(mesh, meshToSlice, sliceToConvex);
	PreviousMeshToConvexTransform4D = meshToConvex;
	bCollisionDirty = false;
}

void UQHSliceCollision3DComponent::RebuildCollisionData(UTetraMesh4D* InMesh, const FTransform4D& InMeshToSliceTransform4D, const FTransform& InSliceToConvexTransform3D)
{
	QH_PROFILE_SCOPE_PRINTF(TEXT("UQHSliceCollision3DComponent::RebuildCollisionData - %s"), *InMesh->GetName());

	FKConvexElem convexElement;
	const TArray<int32>& indices = InMesh->GetSimplexCellVertexIndices();
	const TArray<FVector4>& vertices = InMesh->GetVertexValues();
	const bool hasConvex = UQHPhysicsLibrary::SliceTetraMeshConvexHullW0(indices.Num() / 4,
		vertices.GetData(), indices.GetData(), InMeshToSliceTransform4D, convexElement, InSliceToConvexTransform3D);

	// UObject/body changes stay on the game thread. CPU slicing can be moved to workers separately.
	ClearCollisionData();
	if (!hasConvex)
	{
		return;
	}
	TArray<Chaos::FConvex::FVec3Type> convexVertices;
	convexVertices.Reserve(convexElement.VertexData.Num());
	for (const FVector& vertex : convexElement.VertexData)
	{
		convexVertices.Add(Chaos::FConvex::FVec3Type(vertex.X, vertex.Y, vertex.Z));
	}
	Chaos::FConvexPtr convex = new Chaos::FConvex(convexVertices, 0.0f);
	if (!convex->IsValidGeometry())
	{
		return;
	}
	convex->SetDoCollide(true);
	convexElement.UpdateElemBox();
	LocalCollisionBounds = FBoxSphereBounds(convexElement.ElemBox);

	if (!BodySetup)
	{
		BodySetup = NewObject<UBodySetup>(this);
		BodySetup->BodySetupGuid = FGuid::NewGuid();
		BodySetup->bGenerateMirroredCollision = false;
		BodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;
	}
	// FKConvexElem's copy constructor omits its Chaos object, so install it after adding the element.
	FKConvexElem& bodyConvex = BodySetup->AggGeom.ConvexElems.AddDefaulted_GetRef();
	bodyConvex.VertexData = MoveTemp(convexElement.VertexData);
	bodyConvex.IndexData = MoveTemp(convexElement.IndexData);
	bodyConvex.ElemBox = convexElement.ElemBox;
	bodyConvex.SetConvexMeshObject(MoveTemp(convex), FKConvexElem::EConvexDataUpdateMethod::UpdateConvexDataOnlyIfMissing);
	BodySetup->bCreatedPhysicsMeshes = true;
	UpdateBounds();
	CreatePhysicsState();
}
