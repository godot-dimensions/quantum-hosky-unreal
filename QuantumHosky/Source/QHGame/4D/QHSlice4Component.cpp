#include "QHSlice4Component.h"

#include "QHGame/4D/QHSlice4MemberComponent.h"
#include "QHGame/Game/QHWorldSettings.h"
#include "Unreal4DCore/Scene/Components/Scene4DComponent.h"
#include "Engine/World.h"

UQHSlice4Component* UQHSlice4Component::FindSlice4ComponentForScene(UScene4DComponent* InComponent)
{
	if (!IsValid(InComponent))
	{
		return nullptr;
	}
	for (UScene4DComponent* component = InComponent; component != nullptr; component = component->GetParent4DComponent())
	{
		AActor* actor = component->GetOwner();
		if (actor == nullptr)
		{
			continue;
		}
		if (UQHSlice4MemberComponent* member = actor->FindComponentByClass<UQHSlice4MemberComponent>())
		{
			if (UQHSlice4Component* slice = member->GetOwnerSlice4(); IsValid(slice))
			{
				return slice;
			}
			// Editor lookups can run before BeginPlay assigns the member's default slice.
			// Use already-loaded actor references so lookup does not load another map.
			if (AActor* sliceActor = member->OwnerSlice4Actor.Get())
			{
				if (UQHSlice4Component* slice = sliceActor->FindComponentByClass<UQHSlice4Component>(); IsValid(slice))
				{
					return slice;
				}
			}
		}
	}
	UWorld* world = InComponent->GetWorld();
	if (world == nullptr)
	{
		return nullptr;
	}
	if (const AQHWorldSettings* worldSettings = Cast<AQHWorldSettings>(world->GetWorldSettings()))
	{
		return worldSettings->GetDefaultSlice4Component();
	}
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
const FQHSlice4& UQHSlice4Component::GetSlice4() const
{
	return Slice4;
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4Component::SetSlice4(const FQHSlice4& InSlice)
{
	if (Slice4 != InSlice)
	{
		SetSlice4_Implementation(InSlice);
	}
}

////////////////////////////////////////////////////////////////////////////////
const TArray<UQHSlice4MemberComponent*>& UQHSlice4Component::GetSlice4Members() const
{
	return Slice4Members;
}

FTransform4D UQHSlice4Component::GetSliceTransform4D() const
{
	return Slice4.GetSliceTransform4D();
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4Component::RegisterMember(UQHSlice4MemberComponent* InSlice4MemberComponent)
{
	check(::IsValid(InSlice4MemberComponent));

	if (ensureAlways(!Slice4Members.Contains(InSlice4MemberComponent)))
	{
		ensureAlways(InSlice4MemberComponent->GetOwnerSlice4() == this);

		Slice4Members.Add(InSlice4MemberComponent);

		OnMembershipChanged.Broadcast(this, InSlice4MemberComponent, true);
	}
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4Component::UnregisterMember(UQHSlice4MemberComponent* InSlice4MemberComponent)
{
	check(InSlice4MemberComponent);

	if (ensureAlways(Slice4Members.Contains(InSlice4MemberComponent)))
	{
		ensureAlways(InSlice4MemberComponent->GetOwnerSlice4() == this);

		Slice4Members.RemoveSingleSwap(InSlice4MemberComponent);

		OnMembershipChanged.Broadcast(this, InSlice4MemberComponent, false);
	}
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4Component::SetSlice4_Implementation(const FQHSlice4& InSlice)
{
	const FQHSlice4 OldSlice = Slice4;
	Slice4 = InSlice;

	OnSlice4Changed.Broadcast(this, Slice4, OldSlice);
}

#if WITH_EDITOR

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4Component::PostEditChangeProperty(FPropertyChangedEvent& InEvent)
{
	Super::PostEditChangeProperty(InEvent);

	// TODO: Allow editing of the FQHSlice4 directly via a custom property editor.
	{
		FQHSlice4 NewSlice4 = Slice4;
		NewSlice4.SetRotation4(Slice4Rotation);

		SetSlice4_Implementation(NewSlice4);
	}
}

#endif //~ WITH_EDITOR

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4Component::EndPlay(const EEndPlayReason::Type InReason)
{
	Super::EndPlay(InReason);

	for (int32 i = (Slice4Members.Num() - 1); i >= 0; --i)
	{
		Slice4Members[i]->SetOwnerSlice4(nullptr);
	}
}
