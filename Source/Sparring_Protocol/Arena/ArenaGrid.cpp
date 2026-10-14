#include "Arena/ArenaGrid.h"

void FArenaGrid::Initialize(const FIntVector& InSize, float InCellSize, const FVector& InOrigin)
{
	Size = FIntVector(FMath::Max(InSize.X, 0), FMath::Max(InSize.Y, 0), FMath::Max(InSize.Z, 0));
	CellSize = FMath::Max(InCellSize, 1.f);
	Origin = InOrigin;
	Cells.Init(EArenaCellState::Free, Size.X * Size.Y * Size.Z);
}

bool FArenaGrid::IsInBounds(const FIntVector& Cell) const
{
	return Cell.X >= 0 && Cell.X < Size.X
		&& Cell.Y >= 0 && Cell.Y < Size.Y
		&& Cell.Z >= 0 && Cell.Z < Size.Z;
}

EArenaCellState FArenaGrid::GetState(const FIntVector& Cell) const
{
	return IsInBounds(Cell) ? Cells[ToIndex(Cell)] : EArenaCellState::OutOfBounds;
}

void FArenaGrid::SetState(const FIntVector& Cell, EArenaCellState NewState)
{
	if (IsInBounds(Cell))
	{
		Cells[ToIndex(Cell)] = NewState;
	}
}

FVector FArenaGrid::CellToWorld(const FIntVector& Cell) const
{
	return Origin + FVector((Cell.X + 0.5f) * CellSize, (Cell.Y + 0.5f) * CellSize, (Cell.Z + 0.5f) * CellSize);
}

FIntVector FArenaGrid::WorldToCell(const FVector& WorldLocation) const
{
	const FVector Local = (WorldLocation - Origin) / CellSize;
	return FIntVector(FMath::FloorToInt(Local.X), FMath::FloorToInt(Local.Y), FMath::FloorToInt(Local.Z));
}

FBox FArenaGrid::GetWorldBounds() const
{
	return FBox(Origin, Origin + FVector(Size.X * CellSize, Size.Y * CellSize, Size.Z * CellSize));
}

int32 FArenaGrid::ToIndex(const FIntVector& Cell) const
{
	return Cell.X + Size.X * (Cell.Y + Size.Y * Cell.Z);
}
