#include "QHGameModule.h"

#include "QHGame/4D/QHScene4Component.h"
#include "Unreal4DCore/Render/RenderingServer4D.h"
#include "Unreal4DCore/Scene/Components/Scene4DComponent.h"

#include "Components/LineBatchComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

#include "Modules/ModuleManager.h"

//------------------------------------------------------------------------------

class FQHGameModule : public FDefaultGameModuleImpl
{
public:
	void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();
		// TODO: Remove this migration exception once QH no longer attaches 4D components under 3D parents.
		UScene4DComponent::bReportNon4DParentErrors = false;
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FQHGameModule, QHGame, "QHGame");

DEFINE_LOG_CATEGORY(LogQHGame);

////////////////////////////////////////////////////////////////////////////////
UScene4DComponent* UQHGameLibrary::GetActor4DComponent(const AActor* InActor)
{
	if (InActor == nullptr)
	{
		return nullptr;
	}
	return Cast<UScene4DComponent>(InActor->GetRootComponent());
}

////////////////////////////////////////////////////////////////////////////////
FTransform4D UQHGameLibrary::GetActorLocalTransform4D(const AActor* InActor)
{
	UScene4DComponent* actor4DComponent = GetActor4DComponent(InActor);
	if (actor4DComponent)
	{
		return actor4DComponent->GetLocalTransform4D();
	}
	return FTransform4D::Identity();
}

////////////////////////////////////////////////////////////////////////////////
FVector4 UQHGameLibrary::GetActorLocalPosition4D(const AActor* InActor)
{
	UScene4DComponent* actor4DComponent = GetActor4DComponent(InActor);
	if (actor4DComponent)
	{
		return actor4DComponent->GetLocalPosition4D();
	}
	return FVector4::Zero();
}

////////////////////////////////////////////////////////////////////////////////
FEuler4D UQHGameLibrary::GetActorLocalRotationDegrees4D(const AActor* InActor)
{
	UScene4DComponent* actor4DComponent = GetActor4DComponent(InActor);
	if (actor4DComponent)
	{
		return actor4DComponent->GetLocalRotationDegrees4D();
	}
	return FEuler4D::Zero();
}

////////////////////////////////////////////////////////////////////////////////
FTransform4D UQHGameLibrary::GetActorTransform4(const AActor* InActor)
{
	FTransform4D Result;

	if (InActor)
	{
		if (UQHScene4Component* Scene4Component = InActor->FindComponentByClass<UQHScene4Component>())
		{
			Result = Scene4Component->GetTransform4();
		}
		else
		{
			const FTransform Transform3 = InActor->GetActorTransform();
			const FRotator Rotation3 = Transform3.GetRotation().Rotator();
			// Actors without UQHScene4Component use their ordinary 3D actor transform.
			// Keep the old 3D-to-4D conversion's reversed roll/yaw signs.
			const FEuler4D Rotation4(
				-FMath::DegreesToRadians(Rotation3.Roll),
				 FMath::DegreesToRadians(Rotation3.Pitch),
				-FMath::DegreesToRadians(Rotation3.Yaw));
			const FVector4 scale4D = FVector4(Transform3.GetScale3D(), 1.0);
			// Regular TRS: scale locally, then rotate, then translate.
			Result = FTransform4D(
				UBasis4D::Basis4D_ScaledLocal(Rotation4.RadiansToBasis(), scale4D),
				FVector4(Transform3.GetLocation(), 0.0));
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
FVector4 UQHGameLibrary::GetActorLocation4(const AActor* InActor)
{
	return UQHGameLibrary::GetActorTransform4(InActor).Origin;
}

////////////////////////////////////////////////////////////////////////////////
FEuler4D UQHGameLibrary::GetActorRotation4(const AActor* InActor)
{
	return UQHGameLibrary::GetActorTransform4(InActor).GetRotation();
}

////////////////////////////////////////////////////////////////////////////////
FVector4 UQHGameLibrary::Vector3DTo4D(const UObject* InWorldContext, const FVector& InVector3D)
{
	FVector4 Result = FVector4::Zero();

	if (const UWorld* World = GEngine->GetWorldFromContextObject(InWorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (URenderingServer4D* RenderingServer = World->GetSubsystem<URenderingServer4D>())
		{
			Result = RenderingServer->Vector3DTo4D(InVector3D);
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
FVector UQHGameLibrary::Vector4DTo3D(const UObject* InWorldContext, const FVector4& InVector4D)
{
	FVector Result = FVector::ZeroVector;

	if (const UWorld* World = GEngine->GetWorldFromContextObject(InWorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (URenderingServer4D* RenderingServer = World->GetSubsystem<URenderingServer4D>())
		{
			Result = RenderingServer->Position4DTo3D(InVector4D);
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
FVector4 UQHGameLibrary::Position3DTo4D(const UObject* InWorldContext, const FVector& InPosition3D)
{
	FVector4 Result = FVector4::Zero();

	if (const UWorld* World = GEngine->GetWorldFromContextObject(InWorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (URenderingServer4D* RenderingServer = World->GetSubsystem<URenderingServer4D>())
		{
			Result = RenderingServer->Position3DTo4D(InPosition3D);
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
FVector UQHGameLibrary::Position4DTo3D(const UObject* InWorldContext, const FVector4& InPosition4D)
{
	FVector Result = FVector::ZeroVector;

	if (const UWorld* World = GEngine->GetWorldFromContextObject(InWorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (URenderingServer4D* RenderingServer = World->GetSubsystem<URenderingServer4D>())
		{
			Result = RenderingServer->Position4DTo3D(InPosition4D);
		}
	}

	return Result;
}

FVector4 UQHGameLibrary::Position4DToSlice4D(const UObject* InWorldContext, const FVector4& InPosition4D)
{
	FVector4 Result = FVector4::Zero();

	if (const UWorld* World = GEngine->GetWorldFromContextObject(InWorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (URenderingServer4D* RenderingServer = World->GetSubsystem<URenderingServer4D>())
		{
			Result = RenderingServer->Position4DToSlice4D(InPosition4D);
		}
	}

	return Result;
}

float UQHGameLibrary::SliceDistance4D(const UObject* InWorldContext, const FVector4& InPosition4D)
{
	float Result = 0.0f;

	if (const UWorld* World = GEngine->GetWorldFromContextObject(InWorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (URenderingServer4D* RenderingServer = World->GetSubsystem<URenderingServer4D>())
		{
			Result = RenderingServer->SliceDistance4D(InPosition4D);
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
UActorComponent* UQHGameLibrary::FindComponent(const AActor* InActor, const TSubclassOf<UActorComponent>& OutComponentType)
{
	UActorComponent* Result = nullptr;

	if (InActor)
	{
		// Search the actor
		Result = InActor->FindComponentByClass(OutComponentType);

		// Recursively search the actor's controller
		if (!Result)
		{
			if (const APawn* Pawn = Cast<APawn>(InActor))
			{
				Result = UQHGameLibrary::FindComponent(Pawn->GetController(), OutComponentType);
			}
		}

		// Recursively search the actor's instigator
		if (!Result)
		{
			Result = UQHGameLibrary::FindComponent(InActor->GetInstigator(), OutComponentType);
		}
	}

	return Result;
}

void UQHGameLibrary::DrawLine(const UObject* InWorldContext, const FVector& InStart, const FVector& InEnd, const FColor& InColor, const float InDuration, const float InThickness)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(InWorldContext, EGetWorldErrorMode::ReturnNull))
	{
		if (ULineBatchComponent* LineBatcher = World->GetLineBatcher(UWorld::ELineBatcherType::World))
		{
			LineBatcher->DrawLine(InStart, InEnd, InColor, 0, InThickness, InDuration);
		}
	}
}
