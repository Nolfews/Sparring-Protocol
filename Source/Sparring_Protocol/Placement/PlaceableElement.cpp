#include "Placement/PlaceableElement.h"
#include "Components/StaticMeshComponent.h"

APlaceableElement::APlaceableElement()
{
	PrimaryActorTick.bCanEverTick = false;

	// The root stays a plain scene component: spawning with a transform overwrites the
	// root's scale, so the mesh scale must live on a child component.
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
}

void APlaceableElement::Initialize(UPlaceableDefinition* InDefinition, EPlacementOwner InOwner, const TArray<FIntVector>& InCells)
{
	Definition = InDefinition;
	PlacementOwner = InOwner;
	OccupiedCells = InCells;
}

void APlaceableElement::BeginPlay()
{
	Super::BeginPlay();
	OnOwnerAssigned(PlacementOwner);
}
