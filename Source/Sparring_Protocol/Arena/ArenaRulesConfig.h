#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ArenaRulesConfig.generated.h"

/** Data Asset holding the arena layout and the building rules. */
UCLASS(BlueprintType)
class SPARRING_PROTOCOL_API UArenaRulesConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Number of cells along X. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "1"))
	int32 GridSizeX = 20;

	/** Number of cells along Y. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "1"))
	int32 GridSizeY = 20;

	/** Number of height levels (1 = flat arena). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "1"))
	int32 GridLevels = 4;

	/** Cell edge length in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "50"))
	float CellSize = 200.f;

	/** Elements the player may place at the start of a run. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "0"))
	int32 StartCount = 3;

	/** Extra elements allowed per round won. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "0"))
	int32 GrowthPerRound = 1;

	/** Maximum number of elements on the map at the same time. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "0"))
	int32 HardCap = 10;

	/** No building within this radius (in cells) around a spawn point. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "0"))
	int32 SpawnExclusionRadius = 3;

	UFUNCTION(BlueprintPure, Category = "Building")
	int32 GetMaxAllowed(int32 RoundsWon) const
	{
		return FMath::Min(StartCount + RoundsWon * GrowthPerRound, HardCap);
	}
};
