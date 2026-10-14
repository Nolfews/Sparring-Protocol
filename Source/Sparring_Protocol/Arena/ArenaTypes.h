#pragma once

#include "CoreMinimal.h"
#include "ArenaTypes.generated.h"

/** State of a single grid cell. */
UENUM(BlueprintType)
enum class EArenaCellState : uint8
{
	Free,
	Occupied,
	SpawnExclusion,
	/** Never stored: returned by the grid when a cell is outside the arena. */
	OutOfBounds
};

/** Who owns a placed element (used for quotas and tinting later on). */
UENUM(BlueprintType)
enum class EPlacementOwner : uint8
{
	Player,
	Enemy
};
