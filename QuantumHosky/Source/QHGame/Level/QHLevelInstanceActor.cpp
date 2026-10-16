#include "QHGame/Level/QHLevelInstanceActor.h"

#include "QHGame/Level/QHLevelInstanceComponent.h"
#include "QHGame/Level/QHLevelInstanceEditorInstanceActor.h"

#include "LevelInstance/LevelInstanceComponent.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "UObject/FortniteMainBranchObjectVersion.h"

#if WITH_EDITOR
#include "Cooker/CookDependency.h"
#include "Cooker/CookEvents.h"
#include "UObject/ObjectSaveContext.h"
#include "WorldPartition/LevelInstance/LevelInstanceActorDesc.h"
#include "LevelInstance/LevelInstanceEditorPivotActor.h"
#include "Misc/MessageDialog.h"
#endif

#define LOCTEXT_NAMESPACE "QHLevelInstanceActor"

AQHLevelInstance::AQHLevelInstance()
	: LevelInstanceActorGuid(this)
	, LevelInstanceActorImpl(this)
{
	RootComponent = LevelInstanceComponent = CreateDefaultSubobject<UQHLevelInstanceComponent>(TEXT("Root4DComponent"));
	RootComponent->Mobility = EComponentMobility::Static;

#if WITH_EDITORONLY_DATA
	SetDesiredRuntimeBehavior(ELevelInstanceRuntimeBehavior::Partitioned);
#endif
}

void AQHLevelInstance::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FFortniteMainBranchObjectVersion::GUID);

	Super::Serialize(Ar);
	Ar << LevelInstanceActorGuid;

#if WITH_EDITORONLY_DATA
	if (Ar.GetPortFlags() & PPF_DuplicateForPIE)
	{
		// Remove PIE Prefix in case the LevelInstance was part of the DuplicateWorldForPIE
		// This can happen if the level (WorldAsset) is part of the world's levels.
		// ULevelStreaming::RenameForPIE will call FSoftObjectPath::AddPIEPackageName which will
		// force this softobjectpath to be processed by FSoftObjectPath::FixupForPIE (even one that comes from a level instance).
		WorldAsset = FSoftObjectPath(UWorld::RemovePIEPrefix(WorldAsset.ToString()));
	}

	// Level instances were previously always considered HLOD relevant.
	if (Ar.IsLoading() && Ar.CustomVer(FFortniteMainBranchObjectVersion::GUID) < FFortniteMainBranchObjectVersion::ResetLevelInstanceHLODRelevancy)
	{
		bEnableAutoLODGeneration = true;
	}
#endif
}

void AQHLevelInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.Condition = COND_InitialOnly;
	DOREPLIFETIME_WITH_PARAMS_FAST(AQHLevelInstance, LevelInstanceSpawnGuid, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(AQHLevelInstance, CookedWorldAsset, Params);
}

void AQHLevelInstance::PostNetInit()
{
	Super::PostNetInit();

	if (ensure(!LevelInstanceActorGuid.IsValid()))
	{
		check(LevelInstanceSpawnGuid.IsValid());
		LevelInstanceActorGuid.ActorGuid = LevelInstanceSpawnGuid;
		LevelInstanceActorImpl.RegisterLevelInstance();
	}
}

void AQHLevelInstance::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	if (GetLocalRole() == ENetRole::ROLE_Authority && GetWorld()->IsGameWorld())
	{
#if !WITH_EDITOR
		// If the level instance was spawned, not loaded
		LevelInstanceActorGuid.AssignIfInvalid();
#endif
		LevelInstanceSpawnGuid = LevelInstanceActorGuid.GetGuid();
	}

	if (LevelInstanceActorGuid.IsValid())
	{
		LevelInstanceActorImpl.RegisterLevelInstance();
	}
}

void AQHLevelInstance::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();

	LevelInstanceActorImpl.UnregisterLevelInstance();
}

bool AQHLevelInstance::IsLoadingEnabled() const
{
	return LevelInstanceActorImpl.IsLoadingEnabled();
}

const TSoftObjectPtr<UWorld>& AQHLevelInstance::GetWorldAsset() const
{
#if WITH_EDITORONLY_DATA
	const TSoftObjectPtr<UWorld> &Result = GetPropertyOverrideAsset() ? GetPropertyOverrideAsset()->GetWorldAsset() : WorldAsset;
	if (!Result.IsNull())
	{
		return Result;
	}
#endif
	return CookedWorldAsset;
}

const FLevelInstanceID& AQHLevelInstance::GetLevelInstanceID() const
{
	return LevelInstanceActorImpl.GetLevelInstanceID();
}

bool AQHLevelInstance::HasValidLevelInstanceID() const
{
	return LevelInstanceActorImpl.HasValidLevelInstanceID();
}

const FGuid& AQHLevelInstance::GetLevelInstanceGuid() const
{
	return LevelInstanceActorGuid.GetGuid();
}

void AQHLevelInstance::OnLevelInstanceLoaded()
{
	LevelInstanceActorImpl.OnLevelInstanceLoaded();

	if (ULevel* Level = GetLoadedLevel())
	{
		Level->OnApplyLevelTransform.AddWeakLambda(this, [this, Level](const FTransform& InTransform)
		{
			ApplyLevelTransform4D(Level, LevelInstanceComponent->GetGlobalTransform4D());

			Level->OnApplyLevelTransform.RemoveAll(this);
		});
	}
}

#if WITH_EDITOR

void AQHLevelInstance::SetPropertyOverrideAsset(ULevelInstancePropertyOverrideAsset* InPropertyOverrideAsset)
{
	if (PropertyOverrides != InPropertyOverrideAsset)
	{
		Modify();
		PropertyOverrides = InPropertyOverrideAsset;
	}
}

ILevelInstanceComponentInterface* AQHLevelInstance::GetLevelInstanceComponent() const
{
	return Cast<ILevelInstanceComponentInterface>(RootComponent);
}

AActor* AQHLevelInstance::CreateLevelInstanceEditorInstance(ILevelInstanceInterface* LevelInstance, ULevel* LoadedLevel) const
{
	return AQHLevelInstanceEditorInstanceActor::Create(LevelInstance, LoadedLevel);
}

// TODO: QHLevelInstance
TSubclassOf<AActor> AQHLevelInstance::GetEditorPivotClass() const
{
	return ALevelInstancePivot::StaticClass();
}

void AQHLevelInstance::AdjustPivotOnCreation()
{
	// TODO: QHLevelInstance; this currently breaks when creating level instance from selection
	LevelInstanceComponent->SetGlobalPosition4D(FVector4(GetActorLocation(), 0.0f));
}

bool AQHLevelInstance::SupportsPartialEditorLoading() const
{
	return ILevelInstanceInterface::SupportsPartialEditorLoading() && LevelInstanceActorImpl.SupportsPartialEditorLoading();
}

bool AQHLevelInstance::SupportsPropertyOverrides() const
{
	return LevelInstanceActorImpl.SupportsPropertyOverrides();
}

ULevelInstancePropertyOverrideAsset* AQHLevelInstance::GetPropertyOverrideAsset() const
{
	return SupportsPropertyOverrides() ? PropertyOverrides : nullptr;
}

TUniquePtr<FWorldPartitionActorDesc> AQHLevelInstance::CreateClassActorDesc() const
{
	return TUniquePtr<FWorldPartitionActorDesc>(new FLevelInstanceActorDesc());
}

AQHLevelInstance::FOnLevelInstanceActorPostLoad AQHLevelInstance::OnLevelInstanceActorPostLoad;

void AQHLevelInstance::PostLoad()
{
	Super::PostLoad();

	LevelInstanceComponent = Cast<UQHLevelInstanceComponent>(RootComponent);
	OnLevelInstanceActorPostLoad.Broadcast(this);

#if WITH_EDITORONLY_DATA
	if (IsRunningCookCommandlet() && ShouldCookWorldAsset())
	{
		CookedWorldAsset = GetWorldAsset();
	}
#endif
}

void AQHLevelInstance::OnCookEvent(UE::Cook::ECookEvent CookEvent, UE::Cook::FCookEventContext& CookContext)
{
	Super::OnCookEvent(CookEvent, CookContext);

	if (CookEvent == UE::Cook::ECookEvent::PlatformCookDependencies)
	{
		FName WorldPackageName = FName(GetWorldAsset().GetLongPackageName());
		if (!WorldPackageName.IsNone())
		{
			CookContext.AddLoadBuildDependency(UE::Cook::FCookDependency::Package(WorldPackageName));
		}
	}
}

bool AQHLevelInstance::ShouldCookWorldAsset() const
{
	// If ALevelInstance actor gets loaded it means it needs to Cook its WorldAsset (World Partition Embedded Level Instances don't get loaded as they aren't runtime relevant)
	// If ALevelInstnace is a template then we only need to Cook its WorldAsset if it's desired runtime behavior is to be Level Streamed
	return !IsTemplate() || GetDesiredRuntimeBehavior() == ELevelInstanceRuntimeBehavior::LevelStreaming;
}

#endif // WITH_EDITOR

void AQHLevelInstance::ApplyLevelTransform4D(ULevel* InLevel, const FTransform4D& InTransform4D) const
{
	// Similar to FLevelUtils::ApplyLevelTransform

	if (InLevel && !InTransform4D.IsEqualApprox(FTransform4D::Identity()))
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (Actor)
			{
				if (UScene4DComponent* Root4DComponent = Cast<UScene4DComponent>(Actor->GetRootComponent()))
				{
					if (!Root4DComponent->GetAttachParent())
					{
						Root4DComponent->SetGlobalTransform4D(InTransform4D * Root4DComponent->GetGlobalTransform4D());

						Actor->MarkNeedsRecomputeBoundsOnceForGame();
					}
				}
			}
		}
	}
}

#if WITH_EDITOR

void AQHLevelInstance::PreEditUndo()
{
	LevelInstanceActorImpl.PreEditUndo([this]() { Super::PreEditUndo(); });
}

void AQHLevelInstance::PostEditUndo(TSharedPtr<ITransactionObjectAnnotation> TransactionAnnotation)
{
	LevelInstanceActorImpl.PostEditUndo(TransactionAnnotation, [this](TSharedPtr<ITransactionObjectAnnotation> InTransactionAnnotation) { Super::PostEditUndo(InTransactionAnnotation); });
}

void AQHLevelInstance::PostEditUndo()
{
	LevelInstanceActorImpl.PostEditUndo([this]() { Super::PostEditUndo(); });
}

void AQHLevelInstance::PreEditChange(FProperty* PropertyThatWillChange)
{
	const bool bWorldAssetChange = PropertyThatWillChange && (PropertyThatWillChange->GetFName() == GET_MEMBER_NAME_CHECKED(AQHLevelInstance, WorldAsset));
	LevelInstanceActorImpl.PreEditChange(PropertyThatWillChange, bWorldAssetChange, [this](FProperty* Property) { Super::PreEditChange(Property); });
}

void AQHLevelInstance::CheckForErrors()
{
	Super::CheckForErrors();

	LevelInstanceActorImpl.CheckForErrors();
}

bool AQHLevelInstance::SetWorldAsset(TSoftObjectPtr<UWorld> InWorldAsset)
{
	FString Reason;
	if (!ULevelInstanceSubsystem::CanUseWorldAsset(this, InWorldAsset, &Reason))
	{
		//UE_LOG(LogLevelInstance, Warning, TEXT("%s"), *Reason);
		return false;
	}

	WorldAsset = InWorldAsset;
	return true;
}

void AQHLevelInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const bool bWorldAssetChange = PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AQHLevelInstance, WorldAsset);

	LevelInstanceActorImpl.PostEditChangeProperty(PropertyChangedEvent, bWorldAssetChange, [this](FPropertyChangedEvent& Event) { Super::PostEditChangeProperty(Event); });

#if WITH_EDITORONLY_DATA
	const bool bLIBehaviorChange = PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AQHLevelInstance, DesiredRuntimeBehavior);

	if (bLIBehaviorChange)
	{
		LevelInstanceComponent->ShadowDesiredRuntimeBehavior = DesiredRuntimeBehavior;
	}
#endif

}

bool AQHLevelInstance::CanEditChange(const FProperty* Property) const
{
	return Super::CanEditChange(Property) && LevelInstanceActorImpl.CanEditChange(Property);
}

bool AQHLevelInstance::CanEditChangeComponent(const UActorComponent* InComponent, const FProperty* InProperty) const
{
	return Super::CanEditChangeComponent(InComponent, InProperty) && LevelInstanceActorImpl.CanEditChangeComponent(InComponent, InProperty);
}

bool AQHLevelInstance::ResolveSubobject(const TCHAR* SubObjectPath, UObject*& OutObject, bool bLoadIfExists)
{
	return LevelInstanceActorImpl.ResolveSubobject(SubObjectPath, OutObject, bLoadIfExists);
}

void AQHLevelInstance::PostEditImport()
{
	LevelInstanceActorImpl.PostEditImport([this]() { Super::PostEditImport(); });
}

bool AQHLevelInstance::CanDeleteSelectedActor(FText& OutReason) const
{
	return Super::CanDeleteSelectedActor(OutReason) && LevelInstanceActorImpl.CanDeleteSelectedActor(OutReason);
}

void AQHLevelInstance::SetIsTemporarilyHiddenInEditor(bool bIsHidden)
{
	LevelInstanceActorImpl.SetIsTemporarilyHiddenInEditor(bIsHidden, [this](bool bInHidden) { Super::SetIsTemporarilyHiddenInEditor(bInHidden); });
}

bool AQHLevelInstance::SetIsHiddenEdLayer(bool bIsHiddenEdLayer)
{
	return LevelInstanceActorImpl.SetIsHiddenEdLayer(bIsHiddenEdLayer, [this](bool bInHiddenEdLayer) { return Super::SetIsHiddenEdLayer(bInHiddenEdLayer); });
}

void AQHLevelInstance::EditorGetUnderlyingActors(TSet<AActor*>& OutUnderlyingActors) const
{
	Super::EditorGetUnderlyingActors(OutUnderlyingActors);
	LevelInstanceActorImpl.EditorGetUnderlyingActors(OutUnderlyingActors);
}

void AQHLevelInstance::GetStreamingBounds(FBox& OutRuntimeBounds, FBox& OutEditorBounds) const
{
	Super::GetStreamingBounds(OutRuntimeBounds, OutEditorBounds);

	FBox LevelInstanceRuntimeBounds;
	if (LevelInstanceActorImpl.GetBounds(LevelInstanceRuntimeBounds))
	{
		OutRuntimeBounds = LevelInstanceRuntimeBounds;
	}

	FBox LevelInstanceEditorBounds;
	if (LevelInstanceActorImpl.GetEditorBounds(LevelInstanceEditorBounds))
	{
		OutEditorBounds = LevelInstanceEditorBounds;
	}
}

bool AQHLevelInstance::IsUserManaged() const
{
	return Super::IsUserManaged() || LevelInstanceActorImpl.IsUserManaged();
}

bool AQHLevelInstance::ShouldExport()
{
	return Super::ShouldExport() && LevelInstanceActorImpl.ShouldExport();
}

bool AQHLevelInstance::IsLockLocation() const
{
	return Super::IsLockLocation() || LevelInstanceActorImpl.IsLockLocation();
}

bool AQHLevelInstance::IsActorLabelEditable() const
{
	return Super::IsActorLabelEditable() && LevelInstanceActorImpl.IsActorLabelEditable();
}

bool AQHLevelInstance::GetReferencedContentObjects(TArray<UObject*>& Objects) const
{
	Objects.Add(const_cast<AQHLevelInstance*>(this));
	return true;
}

bool AQHLevelInstance::GetSoftReferencedContentObjects(TArray<FSoftObjectPath>& SoftObjects) const
{
	if (GetWorldAsset().ToSoftObjectPath().IsValid())
	{
		SoftObjects.Add(GetWorldAsset().ToSoftObjectPath());
		return true;
	}
	return false;
}

bool AQHLevelInstance::OpenAssetEditor()
{
	FText Reason;
	if (!CanEnterEdit(&Reason))
	{
		FMessageDialog::Open(EAppMsgType::Ok, Reason);
		return false;
	}
	return EnterEdit();
}

bool AQHLevelInstance::EditorCanAttachFrom(const AActor* InChild, FText& OutReason) const
{
	if (IsEditing())
	{
		return true;
	}

	return false;
}

FBox AQHLevelInstance::GetComponentsBoundingBox(bool bNonColliding, bool bIncludeFromChildActors) const
{
	FBox Box = Super::GetComponentsBoundingBox(bNonColliding, bIncludeFromChildActors);

	FBox LevelInstanceBounds;
	if (LevelInstanceActorImpl.GetBounds(LevelInstanceBounds))
	{
		Box += LevelInstanceBounds;
	}

	return Box;
}

void AQHLevelInstance::PushSelectionToProxies()
{
	Super::PushSelectionToProxies();

	LevelInstanceActorImpl.PushSelectionToProxies();
}

void AQHLevelInstance::PushLevelInstanceEditingStateToProxies(bool bInEditingState)
{
	Super::PushLevelInstanceEditingStateToProxies(bInEditingState);

	LevelInstanceActorImpl.PushLevelInstanceEditingStateToProxies(bInEditingState);
}

bool AQHLevelInstance::IsHLODRelevant() const
{
	if (!IsWorldAssetValid())
	{
		return false;
	}

	return Super::IsHLODRelevant();
}

bool AQHLevelInstance::HasHLODRelevantComponents() const
{
	// Assume LIs can provide HLOD relevant components
	return true;
}

void AQHLevelInstance::SetDesiredRuntimeBehavior(ELevelInstanceRuntimeBehavior NewBehavior)
{
	DesiredRuntimeBehavior = NewBehavior;
	LevelInstanceComponent->ShadowDesiredRuntimeBehavior = NewBehavior;
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
