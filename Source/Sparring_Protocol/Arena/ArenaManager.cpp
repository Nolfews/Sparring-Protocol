#include "Arena/ArenaManager.h"
#include "Arena/ArenaRulesConfig.h"
#include "Arena/ArenaSpawnPoint.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
	
DEFINE_LOG_CATEGORY_STATIC(LogArena, Log, All);

AArenaManager::AArenaManager()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorMesh->SetupAttachment(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		FloorMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void AArenaManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildGrid();
}

void AArenaManager::BeginPlay()
{
	Super::BeginPlay();
	RebuildGrid();
}

void AArenaManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugGrid)
	{
		DrawDebugGrid();
	}
#endif
}

void AArenaManager::RebuildGrid()
{
	if (!Rules)
	{
		UE_LOG(LogArena, Warning, TEXT("%s has no Rules asset assigned."), *GetName());
		return;
	}

	const FIntVector Size(Rules->GridSizeX, Rules->GridSizeY, Rules->GridLevels);
	const FVector Center = GetActorLocation();
	const FVector Origin(
		Center.X - Size.X * Rules->CellSize * 0.5f,
		Center.Y - Size.Y * Rules->CellSize * 0.5f,
		Center.Z);

	Grid.Initialize(Size, Rules->CellSize, Origin);
	UpdateFloor();
	MarkSpawnExclusionZones();

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	
	
	if (NavSys)
	{
		// Force le recalcul immédiat de l'ensemble du NavMesh
		NavSys->Build();
	}
}

void AArenaManager::UpdateFloor()
{
	if (!FloorMesh || !Rules)
	{
		return;
	}

	// The engine cube is 100x100x100 cm and centered on its pivot.
	const float Width = Rules->GridSizeX * Rules->CellSize;
	const float Depth = Rules->GridSizeY * Rules->CellSize;

	FloorMesh->SetRelativeLocation(FVector(0.f, 0.f, -FloorThickness * 0.5f));
	FloorMesh->SetRelativeScale3D(FVector(Width / 100.f, Depth / 100.f, FloorThickness / 100.f));
}

void AArenaManager::MarkSpawnExclusionZones()
{
	UWorld* World = GetWorld();
	if (!World || !Rules)
	{
		return;
	}

	const int32 Radius = Rules->SpawnExclusionRadius;
	const FIntVector Size = Grid.GetSize();

	for (TActorIterator<AArenaSpawnPoint> It(World); It; ++It)
	{
		FIntVector SpawnCell = Grid.WorldToCell(It->GetActorLocation());
		SpawnCell.Z = 0; // Only X/Y matter, the exclusion covers every level.

		if (!Grid.IsInBounds(SpawnCell))
		{
			UE_LOG(LogArena, Warning, TEXT("Spawn point %s is outside the arena grid."), *It->GetName());
			continue;
		}

		for (int32 DX = -Radius; DX <= Radius; ++DX)
		{
			for (int32 DY = -Radius; DY <= Radius; ++DY)
			{
				if (DX * DX + DY * DY > Radius * Radius)
				{
					continue;
				}
				for (int32 Z = 0; Z < Size.Z; ++Z)
				{
					Grid.SetState(FIntVector(SpawnCell.X + DX, SpawnCell.Y + DY, Z), EArenaCellState::SpawnExclusion);
				}
			}
		}
	}
}

void AArenaManager::DrawDebugGrid() const
{
	UWorld* World = GetWorld();
	const FIntVector Size = Grid.GetSize();
	if (!World || Size.X <= 0 || Size.Y <= 0)
	{
		return;
	}

	const FVector Origin = Grid.GetOrigin();
	const float Cell = Grid.GetCellSize();
	const float LineZ = Origin.Z + 2.f; // Slightly above the floor to avoid z-fighting.

	for (int32 X = 0; X <= Size.X; ++X)
	{
		const FVector Start(Origin.X + X * Cell, Origin.Y, LineZ);
		DrawDebugLine(World, Start, Start + FVector(0.f, Size.Y * Cell, 0.f), FColor::Cyan);
	}
	for (int32 Y = 0; Y <= Size.Y; ++Y)
	{
		const FVector Start(Origin.X, Origin.Y + Y * Cell, LineZ);
		DrawDebugLine(World, Start, Start + FVector(Size.X * Cell, 0.f, 0.f), FColor::Cyan);
	}

	// Full 3D volume of the grid.
	const FBox Bounds = Grid.GetWorldBounds();
	DrawDebugBox(World, Bounds.GetCenter(), Bounds.GetExtent(), FColor::White);

	// Spawn exclusion cells on level 0.
	for (int32 X = 0; X < Size.X; ++X)
	{
		for (int32 Y = 0; Y < Size.Y; ++Y)
		{
			const FIntVector CellCoord(X, Y, 0);
			if (Grid.GetState(CellCoord) == EArenaCellState::SpawnExclusion)
			{
				const FVector Center = Grid.CellToWorld(CellCoord);
				DrawDebugBox(World, FVector(Center.X, Center.Y, LineZ), FVector(Cell * 0.5f - 2.f, Cell * 0.5f - 2.f, 1.f), FColor::Red);
			}
		}
	}
}
