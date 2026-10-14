#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Arena/ArenaGrid.h"
#include "Arena/ArenaTypes.h"
#include "ArenaManager.generated.h"

class UArenaRulesConfig;
class UStaticMeshComponent;
class AArenaSpawnPoint;
class UPlaceableDefinition;
class APlaceableElement;

/**
 * Owns the arena grid and every placed element.
 * Place one in the level at the CENTER of the arena, on the floor surface,
 * with no rotation and scale 1.
 */
UCLASS()
class SPARRING_PROTOCOL_API AArenaManager : public AActor
{
	GENERATED_BODY()

public:
	AArenaManager();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	// ---- Grid access ---------------------------------------------------------

	const FArenaGrid& GetGrid() const { return Grid; }

	UFUNCTION(BlueprintPure, Category = "Arena")
	FVector CellToWorld(FIntVector Cell) const { return Grid.CellToWorld(Cell); }

	UFUNCTION(BlueprintPure, Category = "Arena")
	FIntVector WorldToCell(FVector WorldLocation) const { return Grid.WorldToCell(WorldLocation); }

	// ---- Build phase (called by the round manager) ---------------------------

	/**
	 * Opens a build phase. Computes the element cap from RoundsWon and the rules asset.
	 * Returns the number of slots the PLAYER may place (0 = the caller can skip the phase).
	 */
	UFUNCTION(BlueprintCallable, Category = "Arena|Build")
	int32 BeginBuildPhase(int32 RoundsWon);

	UFUNCTION(BlueprintCallable, Category = "Arena|Build")
	void EndBuildPhase();

	UFUNCTION(BlueprintPure, Category = "Arena|Build")
	bool IsBuildPhaseActive() const { return bBuildPhaseActive; }

	/** Cap on elements per owner for the current phase. */
	UFUNCTION(BlueprintPure, Category = "Arena|Build")
	int32 GetMaxAllowed() const { return CurrentMaxAllowed; }

	/** Slots currently consumed by this owner's elements. */
	UFUNCTION(BlueprintPure, Category = "Arena|Build")
	int32 GetUsedSlots(EPlacementOwner PlacementOwner) const;

	/** Slots this owner can still place. */
	UFUNCTION(BlueprintPure, Category = "Arena|Build")
	int32 GetSlotsToPlace(EPlacementOwner PlacementOwner) const;

	// ---- Placement (same API for the player and the AI) ----------------------

	/** Checks every rule without placing anything. Cell is the footprint's minimum corner. */
	UFUNCTION(BlueprintPure, Category = "Arena|Placement")
	EPlacementFailReason ValidatePlacement(UPlaceableDefinition* Definition, FIntVector Cell, int32 QuarterTurns, EPlacementOwner PlacementOwner) const;

	UFUNCTION(BlueprintCallable, Category = "Arena|Placement")
	FPlacementResult TryPlace(UPlaceableDefinition* Definition, FIntVector Cell, int32 QuarterTurns, EPlacementOwner PlacementOwner);

	/** Every cell where this definition could be placed right now. */
	UFUNCTION(BlueprintCallable, Category = "Arena|Placement")
	TArray<FIntVector> GetValidSlots(UPlaceableDefinition* Definition, int32 QuarterTurns, EPlacementOwner PlacementOwner) const;

	UFUNCTION(BlueprintPure, Category = "Arena|Placement")
	APlaceableElement* GetElementAtCell(FIntVector Cell) const;

	/** Destroys every element and resets the grid. Call at the start of a new run. */
	UFUNCTION(BlueprintCallable, Category = "Arena|Placement")
	void ClearAllElements();

	// ---- Events --------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Arena|Events")
	FOnElementPlaced OnElementPlaced;

	UPROPERTY(BlueprintAssignable, Category = "Arena|Events")
	FOnBuildPhaseStarted OnBuildPhaseStarted;

	UPROPERTY(BlueprintAssignable, Category = "Arena|Events")
	FOnBuildPhaseFinished OnBuildPhaseFinished;

	// ---- Debug test harness (play in editor, select this actor, use the buttons) ----

	UPROPERTY(EditAnywhere, Category = "Debug|Test")
	TObjectPtr<UPlaceableDefinition> DebugDefinition;

	UPROPERTY(EditAnywhere, Category = "Debug|Test")
	FIntVector DebugCell = FIntVector(10, 10, 0);

	UPROPERTY(EditAnywhere, Category = "Debug|Test")
	int32 DebugQuarterTurns = 0;

	UPROPERTY(EditAnywhere, Category = "Debug|Test")
	int32 DebugRoundsWon = 0;

	UPROPERTY(EditAnywhere, Category = "Debug|Test")
	EPlacementOwner DebugOwner = EPlacementOwner::Player;

	UFUNCTION(CallInEditor, Category = "Debug|Test")
	void Debug_BeginBuildPhase();

	UFUNCTION(CallInEditor, Category = "Debug|Test")
	void Debug_EndBuildPhase();

	UFUNCTION(CallInEditor, Category = "Debug|Test")
	void Debug_TryPlace();

	UFUNCTION(CallInEditor, Category = "Debug|Test")
	void Debug_PlaceRandomValid();

	UFUNCTION(CallInEditor, Category = "Debug|Test")
	void Debug_ClearAll();

	UFUNCTION(CallInEditor, Category = "Debug|Test")
	void Debug_LogState();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	TObjectPtr<UArenaRulesConfig> Rules;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FloorMesh;

	/** Floor thickness in cm (the top face sits at the manager's Z). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena", meta = (ClampMin = "1"))
	float FloorThickness = 50.f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebugGrid = true;

private:
	void RebuildGrid();
	void UpdateFloor();
	void MarkSpawnExclusionZones();
	void DrawDebugGrid() const;
	bool CanRunDebugAction() const;

	FArenaGrid Grid;

	bool bBuildPhaseActive = false;
	int32 CurrentMaxAllowed = 0;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APlaceableElement>> PlacedElements;

	TMap<FIntVector, TWeakObjectPtr<APlaceableElement>> ElementByCell;
};
