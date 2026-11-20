#include "Placement/PlaceableElement.h"
#include "Placement/PlaceableDefinition.h"
#include "Arena/ArenaManager.h"
#include "Engine/DamageEvents.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

APlaceableElement::APlaceableElement()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // Only ticks while the crack animates.

	// The root stays a plain scene component: spawning with a transform overwrites the
	// root's scale, so the mesh scale must live on a child component.
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
}

void APlaceableElement::Initialize(UPlaceableDefinition* InDefinition, EPlacementOwner InOwner, const TArray<FIntVector>& InCells, const FVector& InWorldBoxSize)
{
	Definition = InDefinition;
	PlacementOwner = InOwner;
	OccupiedCells = InCells;
	WorldBoxSize = InWorldBoxSize;

	TotalRounds = InDefinition ? FMath::Max(InDefinition->LifetimeRounds, 0) : 0;
	RoundsRemaining = TotalRounds;

	MaxHealth = InDefinition ? FMath::Max(InDefinition->MaxHealth, 0.f) : 0.f;
	CurrentHealth = MaxHealth;
}

void APlaceableElement::BeginPlay()
{
	Super::BeginPlay();

	// One dynamic material per slot, so this element cracks independently of the others.
	DynamicMaterials.Reset();
	for (int32 Index = 0; Index < Mesh->GetNumMaterials(); ++Index)
	{
		if (UMaterialInstanceDynamic* Dynamic = Mesh->CreateAndSetMaterialInstanceDynamic(Index))
		{
			DynamicMaterials.Add(Dynamic);
		}
	}

	ApplyOwnerTint();
	ApplyCrack(CurrentCrack);
	OnOwnerAssigned(PlacementOwner);
}

void APlaceableElement::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CurrentCrack = FMath::FInterpConstantTo(CurrentCrack, TargetCrack, DeltaSeconds, CrackTransitionSpeed);
	ApplyCrack(CurrentCrack);

	if (FMath::IsNearlyEqual(CurrentCrack, TargetCrack))
	{
		SetActorTickEnabled(false);
	}
}

float APlaceableElement::GetDamageFraction() const
{
	// Two sources of wear, combined: the rounds it has lived through and the hits it has taken.
	const float LifetimeDamage = (TotalRounds > 0)
		? FMath::Clamp(1.f - static_cast<float>(RoundsRemaining) / static_cast<float>(TotalRounds), 0.f, 1.f)
		: 0.f;
	const float HitDamage = 1.f - GetHealthFraction();

	return FMath::Clamp(1.f - (1.f - LifetimeDamage) * (1.f - HitDamage), 0.f, 1.f);
}

float APlaceableElement::GetHealthFraction() const
{
	if (MaxHealth <= 0.f)
	{
		return 1.f;
	}
	return FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f);
}

float APlaceableElement::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bBroken || DamageAmount <= 0.f)
	{
		return 0.f;
	}
	if (MaxHealth <= 0.f)
	{
		ShowDebugMessage(TEXT("hit ignored: indestructible (Max Health is 0 in its definition)"), FColor::Orange);
		return 0.f;
	}

	AArenaManager* Manager = Cast<AArenaManager>(GetOwner());
	if (bIgnoreDamageDuringBuildPhase && Manager && Manager->IsBuildPhaseActive())
	{
		ShowDebugMessage(TEXT("hit ignored: a build phase is active (damage only counts during combat)"), FColor::Orange);
		return 0.f;
	}

	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (Applied <= 0.f)
	{
		return 0.f;
	}

	CurrentHealth = FMath::Max(0.f, CurrentHealth - Applied);
	ShowDebugMessage(FString::Printf(TEXT("-%.0f HP (%.0f / %.0f)"), Applied, CurrentHealth, MaxHealth), FColor::Yellow);

	// The cracks follow the health: they spread with every hit.
	TargetCrack = GetDamageFraction();
	SetActorTickEnabled(true);

	FVector HitLocation = GetActorLocation();
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		HitLocation = static_cast<const FPointDamageEvent&>(DamageEvent).HitInfo.ImpactPoint;
	}
	OnDamaged(Applied, GetHealthFraction(), HitLocation);

	if (CurrentHealth <= 0.f)
	{
		if (Manager)
		{
			Manager->RemoveElement(this); // Frees the cells, plays the break effect, collapses what rests on it.
		}
		else
		{
			Break();
		}
	}
	return Applied;
}

bool APlaceableElement::AgeOneRound()
{
	if (IsPermanent() || bBroken)
	{
		return false;
	}

	RoundsRemaining = FMath::Max(0, RoundsRemaining - 1);
	TargetCrack = GetDamageFraction();
	SetActorTickEnabled(true);
	OnLifetimeChanged(RoundsRemaining, TotalRounds);

	return RoundsRemaining <= 0;
}

void APlaceableElement::Break()
{
	if (bBroken)
	{
		return;
	}
	bBroken = true;

	// Collision off right away: the NavMesh opens up and nothing can hit the dying element.
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	OnBroken();

	if (bHideMeshOnBreak && Mesh)
	{
		Mesh->SetVisibility(false);
	}
	SetLifeSpan(FMath::Max(BreakLifeSpan, 0.1f));
}

void APlaceableElement::ApplyCrack(float Amount)
{
	for (UMaterialInstanceDynamic* Dynamic : DynamicMaterials)
	{
		if (Dynamic)
		{
			Dynamic->SetScalarParameterValue(CrackParameterName, Amount);
		}
	}
}

void APlaceableElement::ShowDebugMessage(const FString& Message, const FColor& Color) const
{
	if (bShowDebugMessages && GEngine)
	{
		// One line per element, updated at each hit instead of scrolling.
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 2.f, Color,
			FString::Printf(TEXT("%s: %s"), *GetName(), *Message));
	}
}

void APlaceableElement::ApplyOwnerTint()
{
	const FLinearColor& Tint = (PlacementOwner == EPlacementOwner::Player) ? PlayerTint : EnemyTint;
	for (UMaterialInstanceDynamic* Dynamic : DynamicMaterials)
	{
		if (Dynamic)
		{
			Dynamic->SetVectorParameterValue(OwnerTintParameterName, Tint);
		}
	}
}
