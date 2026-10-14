#include "Arena/ArenaManager.h"
#include "Arena/ArenaRulesConfig.h"
#include "Arena/ArenaSpawnPoint.h"
#include "Placement/PlaceableDefinition.h"
#include "Placement/PlaceableElement.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogArena, Log, All);

namespace
{
	void ShowDebug(const FString& Message, const FColor& Color = FColor::Green)
	{
		UE_LOG(LogArena, Log, TEXT("%s"), *Message);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.f, Color, Message);
		}
	}

	int32 NormalizeQuarterTurns(const UPlaceableDefinition* Definition, int32 QuarterTurns)
	{
		return (Definition && Definition->bAllowRotation) ? (((QuarterTurns % 4) + 4) % 4) : 0;
	}
}

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

// ---- Grid setup ---------------------------------------------------------------

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

// ---- Build phase --------------------------------------------------------------

int32 AArenaManager::BeginBuildPhase(int32 RoundsWon)
{
	if (!Rules)
	{
		UE_LOG(LogArena, Warning, TEXT("BeginBuildPhase called without a Rules asset."));
		return 0;
	}

	CurrentMaxAllowed = Rules->GetMaxAllowed(FMath::Max(RoundsWon, 0));
	bBuildPhaseActive = true;

	const int32 PlayerSlots = GetSlotsToPlace(EPlacementOwner::Player);
	OnBuildPhaseStarted.Broadcast(PlayerSlots);
	return PlayerSlots;
}

void AArenaManager::EndBuildPhase()
{
	if (!bBuildPhaseActive)
	{
		return;
	}
	bBuildPhaseActive = false;
	OnBuildPhaseFinished.Broadcast();
}

int32 AArenaManager::GetUsedSlots(EPlacementOwner PlacementOwner) const
{
	int32 Used = 0;
	for (const TObjectPtr<APlaceableElement>& Element : PlacedElements)
	{
		if (IsValid(Element) && Element->GetPlacementOwner() == PlacementOwner && Element->GetDefinition())
		{
			Used += Element->GetDefinition()->Cost;
		}
	}
	return Used;
}

int32 AArenaManager::GetSlotsToPlace(EPlacementOwner PlacementOwner) const
{
	return FMath::Max(0, CurrentMaxAllowed - GetUsedSlots(PlacementOwner));
}

// ---- Placement ----------------------------------------------------------------

EPlacementFailReason AArenaManager::ValidatePlacement(UPlaceableDefinition* Definition, FIntVector Cell, int32 QuarterTurns, EPlacementOwner PlacementOwner) const
{
	if (!Definition || !Definition->ElementClass)
	{
		return EPlacementFailReason::InvalidDefinition;
	}
	if (!bBuildPhaseActive)
	{
		return EPlacementFailReason::NotInBuildPhase;
	}

	const FIntVector Footprint = Definition->GetRotatedFootprint(QuarterTurns);

	bool bOutOfBounds = false;
	bool bOccupied = false;
	bool bTooClose = false;

	for (int32 X = 0; X < Footprint.X; ++X)
	{
		for (int32 Y = 0; Y < Footprint.Y; ++Y)
		{
			for (int32 Z = 0; Z < Footprint.Z; ++Z)
			{
				switch (Grid.GetState(FIntVector(Cell.X + X, Cell.Y + Y, Cell.Z + Z)))
				{
				case EArenaCellState::OutOfBounds:    bOutOfBounds = true; break;
				case EArenaCellState::Occupied:       bOccupied = true; break;
				case EArenaCellState::SpawnExclusion: bTooClose = true; break;
				default: break;
				}
			}
		}
	}

	if (bOutOfBounds) return EPlacementFailReason::OutOfBounds;
	if (bOccupied)    return EPlacementFailReason::CellOccupied;
	if (bTooClose)    return EPlacementFailReason::TooCloseToSpawn;

	// Stacking: every cell right below the footprint must be occupied.
	if (Cell.Z > 0)
	{
		for (int32 X = 0; X < Footprint.X; ++X)
		{
			for (int32 Y = 0; Y < Footprint.Y; ++Y)
			{
				if (Grid.GetState(FIntVector(Cell.X + X, Cell.Y + Y, Cell.Z - 1)) != EArenaCellState::Occupied)
				{
					return EPlacementFailReason::NoSupport;
				}
			}
		}
	}

	if (GetSlotsToPlace(PlacementOwner) < Definition->Cost)
	{
		return EPlacementFailReason::NoSlotsLeft;
	}

	// TODO (next step): path check between the two spawns -> EPlacementFailReason::BlocksPath.

	return EPlacementFailReason::None;
}

FPlacementResult AArenaManager::TryPlace(UPlaceableDefinition* Definition, FIntVector Cell, int32 QuarterTurns, EPlacementOwner PlacementOwner)
{
	const int32 Turns = NormalizeQuarterTurns(Definition, QuarterTurns);

	const EPlacementFailReason Reason = ValidatePlacement(Definition, Cell, Turns, PlacementOwner);
	if (Reason != EPlacementFailReason::None)
	{
		return FPlacementResult::MakeFailure(Reason);
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return FPlacementResult::MakeFailure(EPlacementFailReason::InvalidDefinition);
	}

	const FIntVector Footprint = Definition->GetRotatedFootprint(Turns);
	const float CellSize = Grid.GetCellSize();

	TArray<FIntVector> Cells;
	Cells.Reserve(Footprint.X * Footprint.Y * Footprint.Z);
	for (int32 X = 0; X < Footprint.X; ++X)
	{
		for (int32 Y = 0; Y < Footprint.Y; ++Y)
		{
			for (int32 Z = 0; Z < Footprint.Z; ++Z)
			{
				Cells.Add(FIntVector(Cell.X + X, Cell.Y + Y, Cell.Z + Z));
			}
		}
	}

	// The actor pivot sits at the center of the footprint box.
	const FVector Location = Grid.CellToWorld(Cell) + FVector(
		(Footprint.X - 1) * CellSize * 0.5f,
		(Footprint.Y - 1) * CellSize * 0.5f,
		(Footprint.Z - 1) * CellSize * 0.5f);
	const FTransform SpawnTransform(FRotator(0.f, 90.f * Turns, 0.f), Location);

	APlaceableElement* Element = World->SpawnActorDeferred<APlaceableElement>(
		Definition->ElementClass, SpawnTransform, this, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Element)
	{
		return FPlacementResult::MakeFailure(EPlacementFailReason::InvalidDefinition);
	}

	Element->Initialize(Definition, PlacementOwner, Cells);
	Element->FinishSpawning(SpawnTransform);

	for (const FIntVector& OccupiedCell : Cells)
	{
		Grid.SetState(OccupiedCell, EArenaCellState::Occupied);
		ElementByCell.Add(OccupiedCell, Element);
	}
	PlacedElements.Add(Element);

	OnElementPlaced.Broadcast(Element);
	return FPlacementResult::MakeSuccess(Element);
}

TArray<FIntVector> AArenaManager::GetValidSlots(UPlaceableDefinition* Definition, int32 QuarterTurns, EPlacementOwner PlacementOwner) const
{
	TArray<FIntVector> Result;
	const FIntVector Size = Grid.GetSize();

	for (int32 Z = 0; Z < Size.Z; ++Z)
	{
		for (int32 Y = 0; Y < Size.Y; ++Y)
		{
			for (int32 X = 0; X < Size.X; ++X)
			{
				const FIntVector Cell(X, Y, Z);
				if (ValidatePlacement(Definition, Cell, QuarterTurns, PlacementOwner) == EPlacementFailReason::None)
				{
					Result.Add(Cell);
				}
			}
		}
	}
	return Result;
}

APlaceableElement* AArenaManager::GetElementAtCell(FIntVector Cell) const
{
	if (const TWeakObjectPtr<APlaceableElement>* Found = ElementByCell.Find(Cell))
	{
		return Found->Get();
	}
	return nullptr;
}

void AArenaManager::ClearAllElements()
{
	for (TObjectPtr<APlaceableElement>& Element : PlacedElements)
	{
		if (IsValid(Element))
		{
			Element->Destroy();
		}
	}
	PlacedElements.Reset();
	ElementByCell.Reset();
	bBuildPhaseActive = false;

	// Re-initializes every cell to Free and re-marks the spawn exclusion zones.
	RebuildGrid();
}

// ---- Debug harness ------------------------------------------------------------

bool AArenaManager::CanRunDebugAction() const
{
	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		ShowDebug(TEXT("Debug actions only work while playing (PIE). They would spawn actors in the editor level."), FColor::Orange);
		return false;
	}
	return true;
}

void AArenaManager::Debug_BeginBuildPhase()
{
	if (!CanRunDebugAction())
	{
		return;
	}
	const int32 PlayerSlots = BeginBuildPhase(DebugRoundsWon);
	ShowDebug(FString::Printf(TEXT("Build phase started. RoundsWon=%d, MaxAllowed=%d, player slots=%d"),
		DebugRoundsWon, CurrentMaxAllowed, PlayerSlots), FColor::Cyan);
}

void AArenaManager::Debug_EndBuildPhase()
{
	if (!CanRunDebugAction())
	{
		return;
	}
	EndBuildPhase();
	ShowDebug(TEXT("Build phase ended."), FColor::Cyan);
}

void AArenaManager::Debug_TryPlace()
{
	if (!CanRunDebugAction())
	{
		return;
	}

	const FPlacementResult Result = TryPlace(DebugDefinition, DebugCell, DebugQuarterTurns, DebugOwner);
	if (Result.bSuccess)
	{
		ShowDebug(FString::Printf(TEXT("Placed at (%d, %d, %d). Slots left: %d"),
			DebugCell.X, DebugCell.Y, DebugCell.Z, GetSlotsToPlace(DebugOwner)));
	}
	else
	{
		ShowDebug(TEXT("Placement refused: ") + UEnum::GetValueAsString(Result.FailReason), FColor::Red);
	}
}

void AArenaManager::Debug_PlaceRandomValid()
{
	if (!CanRunDebugAction())
	{
		return;
	}

	const TArray<FIntVector> Slots = GetValidSlots(DebugDefinition, DebugQuarterTurns, DebugOwner);
	if (Slots.IsEmpty())
	{
		ShowDebug(TEXT("No valid slot available."), FColor::Red);
		return;
	}

	const FIntVector Chosen = Slots[FMath::RandRange(0, Slots.Num() - 1)];
	const FPlacementResult Result = TryPlace(DebugDefinition, Chosen, DebugQuarterTurns, DebugOwner);
	ShowDebug(FString::Printf(TEXT("Random placement at (%d, %d, %d) among %d valid slots: %s"),
		Chosen.X, Chosen.Y, Chosen.Z, Slots.Num(), Result.bSuccess ? TEXT("OK") : TEXT("FAILED")),
		Result.bSuccess ? FColor::Green : FColor::Red);
}

void AArenaManager::Debug_ClearAll()
{
	if (!CanRunDebugAction())
	{
		return;
	}
	ClearAllElements();
	ShowDebug(TEXT("Arena cleared."), FColor::Cyan);
}

void AArenaManager::Debug_LogState()
{
	if (!CanRunDebugAction())
	{
		return;
	}

	const int32 ValidCount = DebugDefinition ? GetValidSlots(DebugDefinition, DebugQuarterTurns, DebugOwner).Num() : 0;
	ShowDebug(FString::Printf(
		TEXT("Phase active: %s | MaxAllowed: %d | Player slots: %d | Enemy slots: %d | Elements: %d | Valid slots (debug def/owner): %d"),
		bBuildPhaseActive ? TEXT("yes") : TEXT("no"),
		CurrentMaxAllowed,
		GetSlotsToPlace(EPlacementOwner::Player),
		GetSlotsToPlace(EPlacementOwner::Enemy),
		PlacedElements.Num(),
		ValidCount), FColor::Cyan);
}

// ---- Debug drawing ------------------------------------------------------------

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

	for (int32 X = 0; X < Size.X; ++X)
	{
		for (int32 Y = 0; Y < Size.Y; ++Y)
		{
			// Spawn exclusion cells on level 0 (red, flat).
			const FIntVector Ground(X, Y, 0);
			if (Grid.GetState(Ground) == EArenaCellState::SpawnExclusion)
			{
				const FVector Center = Grid.CellToWorld(Ground);
				DrawDebugBox(World, FVector(Center.X, Center.Y, LineZ), FVector(Cell * 0.5f - 2.f, Cell * 0.5f - 2.f, 1.f), FColor::Red);
			}

			// Occupied cells on every level (yellow wire cubes).
			for (int32 Z = 0; Z < Size.Z; ++Z)
			{
				const FIntVector Coord(X, Y, Z);
				if (Grid.GetState(Coord) == EArenaCellState::Occupied)
				{
					DrawDebugBox(World, Grid.CellToWorld(Coord), FVector(Cell * 0.5f - 2.f), FColor::Yellow);
				}
			}
		}
	}
}
