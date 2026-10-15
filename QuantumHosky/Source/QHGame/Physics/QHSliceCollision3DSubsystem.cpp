#include "QHSliceCollision3DSubsystem.h"

#include "QHSliceCollision3DComponent.h"
#include "Unreal4DCore/Render/RenderingServer4D.h"
#include "Unreal4DCore/Scene/Components/Mesh4DComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UQHSliceCollision3DSubsystem::Initialize(FSubsystemCollectionBase& InCollection)
{
	Super::Initialize(InCollection);
	URenderingServer4D* server = InCollection.InitializeDependency<URenderingServer4D>();
	RenderingServer = server;
	server->OnMeshRegistered.AddDynamic(this, &UQHSliceCollision3DSubsystem::OnMeshRegistered);
	server->OnMeshUnregistered.AddDynamic(this, &UQHSliceCollision3DSubsystem::OnMeshUnregistered);
	// Subscribe before enumerating; the pending set also deduplicates repeated notifications.
	for (const TWeakObjectPtr<UMesh4DComponent>& mesh : server->GetRegisteredMeshes())
	{
		if (UMesh4DComponent* component = mesh.Get())
		{
			OnMeshRegistered(component);
		}
	}
	FWorldDelegates::OnWorldPreSendAllEndOfFrameUpdates.AddUObject(this, &UQHSliceCollision3DSubsystem::UpdateMeshes);
}

void UQHSliceCollision3DSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldPreSendAllEndOfFrameUpdates.RemoveAll(this);
	if (URenderingServer4D* server = RenderingServer.Get())
	{
		server->OnMeshRegistered.RemoveDynamic(this, &UQHSliceCollision3DSubsystem::OnMeshRegistered);
		server->OnMeshUnregistered.RemoveDynamic(this, &UQHSliceCollision3DSubsystem::OnMeshUnregistered);
	}
	for (const auto& entry : CollisionComponents)
	{
		if (IsValid(entry.Value))
		{
			entry.Value->DestroyComponent();
		}
	}
	CollisionComponents.Reset();
	PendingMeshes.Reset();
	RenderingServer.Reset();
	Super::Deinitialize();
}

bool UQHSliceCollision3DSubsystem::DoesSupportWorldType(EWorldType::Type InWorldType) const
{
	return InWorldType == EWorldType::Game || InWorldType == EWorldType::PIE
		|| InWorldType == EWorldType::Editor || InWorldType == EWorldType::EditorPreview;
}

UQHSliceCollision3DComponent* UQHSliceCollision3DSubsystem::GetCollisionComponent(UMesh4DComponent* InMesh) const
{
	const TObjectPtr<UQHSliceCollision3DComponent>* component = CollisionComponents.Find(InMesh);
	return component && IsValid(*component) ? component->Get() : nullptr;
}

void UQHSliceCollision3DSubsystem::OnMeshRegistered(UMesh4DComponent* InMesh)
{
	if (IsValid(InMesh) && InMesh->GetWorld() == GetWorld())
	{
		PendingMeshes.Add(InMesh);
	}
}

void UQHSliceCollision3DSubsystem::OnMeshUnregistered(UMesh4DComponent* InMesh)
{
	PendingMeshes.Remove(InMesh);
	if (UQHSliceCollision3DComponent* component = GetCollisionComponent(InMesh))
	{
		// Stop collision immediately, but defer attachment-tree changes until registration callbacks finish.
		component->ClearCollisionData();
	}
}

void UQHSliceCollision3DSubsystem::UpdateMeshes(UWorld* InWorld)
{
	if (InWorld != GetWorld())
	{
		return;
	}
	check(IsInGameThread());
	for (auto iterator = CollisionComponents.CreateIterator(); iterator; ++iterator)
	{
		UMesh4DComponent* source = iterator.Key().Get();
		if (!source || !source->IsRegistered() || !IsValid(iterator.Value()))
		{
			if (IsValid(iterator.Value()))
			{
				iterator.Value()->DestroyComponent();
			}
			iterator.RemoveCurrent();
		}
	}

	const TSet<TWeakObjectPtr<UMesh4DComponent>> pending = MoveTemp(PendingMeshes);
	PendingMeshes.Reset();
	for (const TWeakObjectPtr<UMesh4DComponent>& mesh : pending)
	{
		UMesh4DComponent* source = mesh.Get();
		if (!source || !source->IsRegistered() || !IsValid(source->GetOwner()) || CollisionComponents.Contains(mesh))
		{
			continue;
		}
		UQHSliceCollision3DComponent* component = NewObject<UQHSliceCollision3DComponent>(source->GetOwner(),
			NAME_None, RF_Transient | RF_DuplicateTransient | RF_TextExportTransient);
		component->SetSourceMeshComponent(source);
		component->SetMobility(source->Mobility);
		component->SetupAttachment(source);
		component->RegisterComponentWithWorld(InWorld);
		CollisionComponents.Add(mesh, component);
	}

	if (URenderingServer4D* server = RenderingServer.Get())
	{
		const FTransform4D sliceTransform4D = server->GetCurrentCameraTransform4D();
		const FTransform sliceTransform3D = server->GetCurrentCameraTransform3D();
		for (const auto& entry : CollisionComponents)
		{
			entry.Value->UpdateCollisionForSlice(sliceTransform4D, sliceTransform3D);
		}
	}
}
