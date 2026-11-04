#include "Placement/PlaceableElement.h"
#include "Placement/PlaceableDefinition.h"
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
	if (TotalRounds <= 0)
	{
		return 0.f;
	}
	return FMath::Clamp(1.f - static_cast<float>(RoundsRemaining) / static_cast<float>(TotalRounds), 0.f, 1.f);
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
