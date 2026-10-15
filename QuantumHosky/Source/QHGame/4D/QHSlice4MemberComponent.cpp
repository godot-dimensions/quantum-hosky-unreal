#include "QHSlice4MemberComponent.h"

#include "QHGame/4D/QHSlice4Component.h"
#include "Unreal4DCore/Scene/Components/Scene4DComponent.h"
#include "QHGame/Game/QHWorldSettings.h"
#include "Unreal4DCore/Math/Math4D.h"

////////////////////////////////////////////////////////////////////////////////
UQHSlice4Component* UQHSlice4MemberComponent::GetOwnerSlice4() const
{
	return OwnerSlice4;
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4MemberComponent::SetOwnerSlice4(UQHSlice4Component* InSlice4Component)
{
	if (OwnerSlice4 != InSlice4Component)
	{
		UQHSlice4Component* OldOwner = OwnerSlice4;

		if (OldOwner)
		{
			OldOwner->UnregisterMember(this);
			OldOwner->OnSlice4Changed.RemoveDynamic(this, &UQHSlice4MemberComponent::OnSlice4Changed);
		}

		OwnerSlice4 = InSlice4Component;

		if (OwnerSlice4)
		{
			OwnerSlice4->RegisterMember(this);
			OwnerSlice4->OnSlice4Changed.AddDynamic(this, &UQHSlice4MemberComponent::OnSlice4Changed);
		}

		OnOwnerSlice4Changed.Broadcast(this, OwnerSlice4, OldOwner);

		UpdateSlice4Distance();
	}
}

////////////////////////////////////////////////////////////////////////////////
float UQHSlice4MemberComponent::GetDistanceToOwnerSlice4() const
{
	return Slice4Distance;
}

////////////////////////////////////////////////////////////////////////////////
bool UQHSlice4MemberComponent::IsInsideOwnerSlice4() const
{
	// Slice distances > 0 are 'outside' the slice, and slice distances <= 0 are 'inside' the slice.
	return (Slice4Distance <= 0.0f);
}

////////////////////////////////////////////////////////////////////////////////
float UQHSlice4MemberComponent::GetInsideOwnerSlice4Percent() const
{
	return FMath::Clamp(UMath4D::SafeDivide(-FMath::Min(Slice4Distance, 0.0f), Extents4), 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4MemberComponent::UpdateSlice4Distance()
{
	const bool WasInsideSlice4 = IsInsideOwnerSlice4();

	Slice4Distance = FLT_MAX;

	if (OwnerSlice4 && Scene4Component)
	{
		// Calculate 4D location in slice-space
		const FVector4 Slice4Location = OwnerSlice4->GetSlice4().World4ToSlice4(Scene4Component->GetGlobalPosition4D());

		// All points inside the slice hyperplane are located at W = 0 from the slice's perspective; therefore an object's 'distance' to the slice is the magnitude of its W-component in slice-space.
		// We then intersect the extents of the object with the slice to determine if the object is 'outside' or 'inside' the slice.
		// Slice distances > 0 are 'outside' the slice, and slice distances <= 0 are 'inside' the slice.
		Slice4Distance = (FMath::Abs(Slice4Location.W) - Extents4);
	}

	const bool IsInsideSlice4 = IsInsideOwnerSlice4();

	if (IsInsideSlice4 != WasInsideSlice4)
	{
		OnCrossedSlice.Broadcast(this, IsInsideSlice4);
	}
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4MemberComponent::OnSlice4Changed(UQHSlice4Component* InSlice4Component, const FQHSlice4& InNewSlice, const FQHSlice4& InOldSlice)
{
	UpdateSlice4Distance();
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4MemberComponent::OnTransform4Changed(UScene4DComponent* InScene4Component)
{
	UpdateSlice4Distance();
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4MemberComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();

	Scene4Component = Cast<UScene4DComponent>(Owner->GetRootComponent());
	if (!Scene4Component)
	{
		Scene4Component = Owner->GetComponentByClass<UScene4DComponent>();
	}

	if (Scene4Component)
	{
		Scene4Component->OnTransform4DChangedDelegate.AddDynamic(this, &UQHSlice4MemberComponent::OnTransform4Changed);
	}

	if (AActor* Slice4Actor = OwnerSlice4Actor.LoadSynchronous())
	{
		if (UQHSlice4Component* Slice4Component = Slice4Actor->FindComponentByClass<UQHSlice4Component>())
		{
			SetOwnerSlice4(Slice4Component);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4MemberComponent::OnUnregister()
{
	Super::OnUnregister();

	if (Scene4Component)
	{
		Scene4Component->OnTransform4DChangedDelegate.RemoveDynamic(this, &UQHSlice4MemberComponent::OnTransform4Changed);
	}

	SetOwnerSlice4(nullptr);
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4MemberComponent::BeginPlay()
{
	Super::BeginPlay();

	// Use default slice if none explicitly specified
	if (!OwnerSlice4)
	{
		if (AQHWorldSettings* WorldSettings = Cast<AQHWorldSettings>(GetWorld()->GetWorldSettings()))
		{
			SetOwnerSlice4(WorldSettings->GetDefaultSlice4Component());
		}
	}
}
