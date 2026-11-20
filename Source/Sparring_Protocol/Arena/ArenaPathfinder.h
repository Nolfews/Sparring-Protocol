#pragma once

#include "CoreMinimal.h"

class FArenaGrid;

/**
 * Walkability check on the arena grid. Pure logic: no actors, no NavMesh.
 *
 * It is a 4-connected flood fill on ground level (Z = 0), so a character never
 * has to squeeze through a diagonal gap. With 200 cm cells a one-cell corridor
 * is wide enough for the character capsule.
 * Ramps and upper levels will extend this later.
 */
class SPARRING_PROTOCOL_API FArenaPathfinder
{
public:
	/**
	 * True if Goal can be reached from Start by walking.
	 * ExtraBlockedCells simulates a hypothetical placement: those cells (only Z = 0 ones matter)
	 * are treated as blocked without touching the grid.
	 */
	static bool ArePathConnected(
		const FArenaGrid& Grid,
		const FIntVector& Start,
		const FIntVector& Goal,
		const TArray<FIntVector>& ExtraBlockedCells);
};
