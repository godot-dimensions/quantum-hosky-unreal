#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/BodySetup.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "Unreal4DCore/Scene/Components/Mesh4DComponent.h"
#include "QHGame/Physics/QHSliceCollision3DComponent.h"
#include "QHGame/Physics/QHSliceCollision3DSubsystem.h"
#include "Unreal4DCore/Model/Mesh/Tetra/BoxTetraMesh4D.h"
#include "Unreal4DCore/Render/RenderingServer4D.h"

namespace
{
	UWorld* CreateCollisionTestWorld()
	{
		const UWorld::InitializationValues initialization = UWorld::InitializationValues()
			.AllowAudioPlayback(false).CreateNavigation(false).CreateAISystem(false)
			.CreateFXSystem(false).ShouldSimulatePhysics(false).EnableTraceCollision(true);
		UWorld* world = UWorld::CreateWorld(EWorldType::EditorPreview, false, NAME_None,
			nullptr, true, ERHIFeatureLevel::Num, &initialization);
		GEngine->CreateNewWorldContext(EWorldType::EditorPreview).SetCurrentWorld(world);
		return world;
	}

	void DestroyCollisionTestWorld(UWorld* InWorld)
	{
		GEngine->DestroyWorldContext(InWorld);
		InWorld->DestroyWorld(false);
	}

	UMesh4DComponent* CreateSourceMesh(UWorld* InWorld)
	{
		AActor* actor = InWorld->SpawnActor<AActor>();
		UMesh4DComponent* source = NewObject<UMesh4DComponent>(actor);
		actor->SetRootComponent(source);
		source->SetMobility(EComponentMobility::Movable);
		source->EnableCollision = true;
		source->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		source->SetCollisionResponseToAllChannels(ECR_Block);
		UBoxTetraMesh4D* mesh = NewObject<UBoxTetraMesh4D>(source);
		mesh->SetSize(FVector4(200.0, 200.0, 200.0, 200.0));
		source->Mesh4D = mesh;
		source->RegisterComponent();
		return source;
	}

	void UpdateCollisionTestWorld(UWorld* InWorld)
	{
		FWorldDelegates::OnWorldPreSendAllEndOfFrameUpdates.Broadcast(InWorld);
		// These isolated worlds do not tick; publish pending Chaos changes before world queries.
		InWorld->GetPhysicsScene()->Flush();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FQHSliceCollision3DLifecycle, "QH.Physics.SliceCollision3D.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQHSliceCollision3DLifecycle::RunTest(const FString& Parameters)
{
	UWorld* world = CreateCollisionTestWorld();
	ON_SCOPE_EXIT { DestroyCollisionTestWorld(world); };
	UQHSliceCollision3DSubsystem* subsystem = world->GetSubsystem<UQHSliceCollision3DSubsystem>();
	if (!TestNotNull(TEXT("The QH collision subsystem is created for the world."), subsystem))
	{
		return false;
	}
	UMesh4DComponent* source = CreateSourceMesh(world);
	TestNull(TEXT("Companion creation is deferred until registration finishes."), subsystem->GetCollisionComponent(source));
	UpdateCollisionTestWorld(world);
	UQHSliceCollision3DComponent* companion = subsystem->GetCollisionComponent(source);
	if (!TestNotNull(TEXT("An Unreal4D mesh receives a collision companion."), companion))
	{
		return false;
	}
	TestTrue(TEXT("The companion belongs to the source actor."), companion->GetOwner() == source->GetOwner());
	TestTrue(TEXT("The companion is attached to its source mesh."), companion->GetAttachParent() == source);
	TestTrue(TEXT("Collision hits can resolve the source mesh."), companion->GetSourceMeshComponent() == source);
	TestTrue(TEXT("The generated component is transient."), companion->HasAnyFlags(RF_Transient));
	TestNull(TEXT("The source mesh does not own a body setup."), source->GetBodySetup());
	TestTrue(TEXT("The companion creates Chaos physics state."), companion->IsPhysicsStateCreated());
	TestTrue(TEXT("The companion owns a valid Chaos body."), companion->HasValidPhysicsState());

	world->GetSubsystem<URenderingServer4D>()->RegisterMesh(source);
	UpdateCollisionTestWorld(world);
	TestTrue(TEXT("Repeated registration keeps the same companion."), subsystem->GetCollisionComponent(source) == companion);
	source->UnregisterComponent();
	TestFalse(TEXT("Unregistration removes physics immediately."), companion->IsPhysicsStateCreated());
	source->RegisterComponent();
	UpdateCollisionTestWorld(world);
	TestTrue(TEXT("Same-frame reregistration reuses the companion."), subsystem->GetCollisionComponent(source) == companion);
	TestTrue(TEXT("Reregistration restores collision."), companion->IsPhysicsStateCreated());

	TWeakObjectPtr<UQHSliceCollision3DComponent> oldCompanion = companion;
	source->UnregisterComponent();
	UpdateCollisionTestWorld(world);
	TestFalse(TEXT("An unregistered source's companion is destroyed."), oldCompanion.IsValid());
	TestNull(TEXT("The pairing is removed."), subsystem->GetCollisionComponent(source));
	source->RegisterComponent();
	UpdateCollisionTestWorld(world);
	TestNotNull(TEXT("A later registration creates a new companion."), subsystem->GetCollisionComponent(source));

	UMesh4DComponent* plainSource = CreateSourceMesh(world);
	UpdateCollisionTestWorld(world);
	UQHSliceCollision3DComponent* plainCompanion = subsystem->GetCollisionComponent(plainSource);
	TestNotNull(TEXT("Plain Unreal4D mesh components also receive companions."), plainCompanion);
	TWeakObjectPtr<UQHSliceCollision3DComponent> destroyedSourceCompanion = plainCompanion;
	plainSource->DestroyComponent();
	UpdateCollisionTestWorld(world);
	TestFalse(TEXT("Destroying the source also destroys its companion."), destroyedSourceCompanion.IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FQHSliceCollision3DGeometry, "QH.Physics.SliceCollision3D.Geometry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQHSliceCollision3DGeometry::RunTest(const FString& Parameters)
{
	UWorld* world = CreateCollisionTestWorld();
	ON_SCOPE_EXIT { DestroyCollisionTestWorld(world); };
	UMesh4DComponent* source = CreateSourceMesh(world);
	UpdateCollisionTestWorld(world);
	UQHSliceCollision3DComponent* companion = world->GetSubsystem<UQHSliceCollision3DSubsystem>()->GetCollisionComponent(source);
	if (!TestNotNull(TEXT("The test mesh receives a companion."), companion)
		|| !TestNotNull(TEXT("The sliced mesh creates a body setup."), companion->GetBodySetup())
		|| !TestEqual(TEXT("The body contains one convex hull."), companion->GetBodySetup()->AggGeom.ConvexElems.Num(), 1))
	{
		return false;
	}
	FHitResult hit;
	TestTrue(TEXT("World traces hit the sliced Chaos hull."), world->LineTraceSingleByChannel(hit,
		FVector(-300.0, 0.0, 0.0), FVector(300.0, 0.0, 0.0), ECC_Visibility));
	TestTrue(TEXT("The trace identifies the collision companion."), hit.GetComponent() == companion);
	const auto initialHull = companion->GetBodySetup()->AggGeom.ConvexElems[0].GetChaosConvexMesh();
	UpdateCollisionTestWorld(world);
	TestTrue(TEXT("Unchanged input does not rebuild the hull."),
		companion->GetBodySetup()->AggGeom.ConvexElems[0].GetChaosConvexMesh() == initialHull);

	source->EnableCollision = false;
	UpdateCollisionTestWorld(world);
	TestFalse(TEXT("Disabling collision removes the Chaos body."), companion->IsPhysicsStateCreated());
	source->EnableCollision = true;
	UpdateCollisionTestWorld(world);
	TestTrue(TEXT("Reenabling collision restores the body."), companion->IsPhysicsStateCreated());
	source->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UpdateCollisionTestWorld(world);
	TestFalse(TEXT("The source's 3D collision toggle is respected."), companion->IsPhysicsStateCreated());
	source->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	UpdateCollisionTestWorld(world);

	URenderingServer4D* rendering = world->GetSubsystem<URenderingServer4D>();
	const auto stationaryHull = companion->GetBodySetup()->AggGeom.ConvexElems[0].GetChaosConvexMesh();
	int32 movementRebuilds = 0;
	for (int32 frame = 0; frame < 32; ++frame)
	{
		const FVector position(frame * 100.0, frame * -40.0, frame * 0.125);
		FTransform4D camera = FTransform4D::Identity();
		camera.Origin = FVector4(position, 0.0);
		rendering->SetOverrideCameraTransforms(camera, FTransform(position));
		UpdateCollisionTestWorld(world);
		movementRebuilds += companion->GetBodySetup()->AggGeom.ConvexElems[0].GetChaosConvexMesh() != stationaryHull;
	}
	TestEqual(TEXT("Compensated XYZ camera motion keeps the existing hull."), movementRebuilds, 0);
	TestTrue(TEXT("World collision remains fixed during XYZ camera motion."), world->LineTraceSingleByChannel(hit,
		FVector(-300.0, 0.0, 0.0), FVector(300.0, 0.0, 0.0), ECC_Visibility));
	FTransform4D slice = FTransform4D::Identity();
	slice.Origin.W = 1000.0;
	rendering->SetOverrideCameraTransforms(slice, FTransform::Identity);
	UpdateCollisionTestWorld(world);
	TestFalse(TEXT("A mesh outside the slice has no Chaos body."), companion->IsPhysicsStateCreated());
	slice.Origin.W = 60.0;
	CastChecked<UBoxTetraMesh4D>(source->Mesh4D)->SetSize(FVector4(20.0, 20.0, 20.0, 20.0));
	UBoxTetraMesh4D* collisionMesh = NewObject<UBoxTetraMesh4D>(source);
	collisionMesh->SetSize(FVector4(200.0, 200.0, 200.0, 200.0));
	source->CollisionMesh4D = collisionMesh;
	rendering->SetOverrideCameraTransforms(slice, FTransform::Identity);
	UpdateCollisionTestWorld(world);
	TestTrue(TEXT("The collision mesh can intersect while the render mesh does not."), companion->IsPhysicsStateCreated());
	TestTrue(TEXT("Bounds come from the collision geometry."), companion->Bounds.BoxExtent.X > 90.0);
	collisionMesh->SetSize(FVector4(400.0, 400.0, 400.0, 400.0));
	UpdateCollisionTestWorld(world);
	TestTrue(TEXT("Collision asset edits invalidate the sliced hull."), companion->Bounds.BoxExtent.X > 190.0);

	rendering->SetOverrideCameraTransforms(FTransform4D::Identity(), FTransform(FVector(700.0, 0.0, 0.0)));
	UpdateCollisionTestWorld(world);
	TestTrue(TEXT("A 3D slice-transform change moves world collision."), world->LineTraceSingleByChannel(hit,
		FVector(400.0, 0.0, 0.0), FVector(1000.0, 0.0, 0.0), ECC_Visibility));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
