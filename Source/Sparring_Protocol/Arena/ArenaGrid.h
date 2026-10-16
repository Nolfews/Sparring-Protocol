#pragma once

#include "CoreMinimal.h"
#include "Arena/ArenaTypes.h"

/**
 * Plain 3D grid (no UObject, no actor dependency).
 * Cell (0,0,0) is the corner at Origin. Z is the height level.
 * Two independent layers per cell: a state (what occupies it) and a
 * "movement blocked" flag (what the path check uses).
 */
class SPARRING_PROTOCOL_API FArenaGrid
{
public:
	void Initialize(const FIntVector& InSize, float InCellSize, const FVector& InOrigin);

	bool IsInBounds(const FIntVector& Cell) const;

	EArenaCellState GetState(const FIntVector& Cell) const;
	void SetState(const FIntVector& Cell, EArenaCellState NewState);

	/** True if characters cannot walk through this cell. Out of bounds counts as blocked. */
	bool IsMovementBlocked(const FIntVector& Cell) const;
	void SetMovementBlocked(const FIntVector& Cell, bool bBlocked);

	/** World location of the cell center. */
	FVector CellToWorld(const FIntVector& Cell) const;

	/** Cell containing a world location. The result may be out of bounds. */
	FIntVector WorldToCell(const FVector& WorldLocation) const;

	FBox GetWorldBounds() const;

	const FIntVector& GetSize() const { return Size; }
	float GetCellSize() const { return CellSize; }
	const FVector& GetOrigin() const { return Origin; }

private:
	int32 ToIndex(const FIntVector& Cell) const;

	FIntVector Size = FIntVector::ZeroValue;
	float CellSize = 100.f;
	FVector Origin = FVector::ZeroVector;
	TArray<EArenaCellState> Cells;
	TArray<uint8> MovementBlocked;
};
