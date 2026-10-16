#pragma once

#include "CoreMinimal.h"
#include "ArenaTypes.generated.h"

class APlaceableElement;

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

/** Who owns a placed element (used for quotas and tinting). */
UENUM(BlueprintType)
enum class EPlacementOwner : uint8
{
	Player,
	Enemy
};

/** Why a placement was refused. Checked in this order, cheapest first. */
UENUM(BlueprintType)
enum class EPlacementFailReason : uint8
{
	None,
	NotInBuildPhase,
	InvalidDefinition,
	OutOfBounds,
	CellOccupied,
	TooCloseToSpawn,
	/** Stacking: the cells below the footprint are not occupied. */
	NoSupport,
	NoSlotsLeft,
	/** A character stands inside the target cells. */
	BlockedByCharacter,
	/** The placement would cut every walkable path between the two spawns. */
	BlocksPath,
	/** No valid cell exists for this element right now. */
	NoValidSlot
};

USTRUCT(BlueprintType)
struct FPlacementResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Placement")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Placement")
	EPlacementFailReason FailReason = EPlacementFailReason::None;

	UPROPERTY(BlueprintReadOnly, Category = "Placement")
	TObjectPtr<APlaceableElement> Element = nullptr;

	static FPlacementResult MakeSuccess(APlaceableElement* InElement)
	{
		FPlacementResult Result;
		Result.bSuccess = true;
		Result.Element = InElement;
		return Result;
	}

	static FPlacementResult MakeFailure(EPlacementFailReason Reason)
	{
		FPlacementResult Result;
		Result.FailReason = Reason;
		return Result;
	}
};

/** Broadcast when an element has been placed (player or AI). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnElementPlaced, APlaceableElement*, Element);

/** Broadcast when a build phase opens. Carries the slots the PLAYER may place. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildPhaseStarted, int32, PlayerSlots);

/** Broadcast when a build phase ends. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuildPhaseFinished);
