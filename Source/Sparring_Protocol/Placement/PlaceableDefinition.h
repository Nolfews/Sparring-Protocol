#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlaceableDefinition.generated.h"

class APlaceableElement;
class UStaticMesh;

/** One Data Asset per placeable element type. Adding an element = adding an asset. */
UCLASS(BlueprintType)
class SPARRING_PROTOCOL_API UPlaceableDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placeable")
	FText DisplayName;

	/** Actor spawned when the element is placed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placeable")
	TSubclassOf<APlaceableElement> ElementClass;

	/** Mesh used by the placement ghost (later). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placeable")
	TObjectPtr<UStaticMesh> PreviewMesh;

	/** Size in cells (X, Y, Z) before rotation. The pivot of the spawned actor must be the CENTER of this box. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	FIntVector Footprint = FIntVector(1, 1, 1);

	/** Allow 90 degree rotations (swaps X and Y of the footprint). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	bool bAllowRotation = true;

	/** Number of slots this element consumes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "1"))
	int32 Cost = 1;

	/** Blocks characters (used by the path check). A slit wall would be false for projectiles only. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	bool bBlocksMovement = true;

	/** Blocks projectiles (for slit walls and similar). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	bool bBlocksProjectiles = true;

	/** 0 = indestructible. Used in the destruction phase. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lifetime", meta = (ClampMin = "0"))
	float MaxHealth = 0.f;

	/** Rounds before the element breaks. 0 = permanent. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lifetime", meta = (ClampMin = "0"))
	int32 LifetimeRounds = 0;

	/** Footprint after applying the rotation (odd quarter turns swap X and Y). */
	UFUNCTION(BlueprintPure, Category = "Placeable")
	FIntVector GetRotatedFootprint(int32 QuarterTurns) const
	{
		const bool bSwap = bAllowRotation && (FMath::Abs(QuarterTurns) % 2 == 1);
		return bSwap ? FIntVector(Footprint.Y, Footprint.X, Footprint.Z) : Footprint;
	}
};
