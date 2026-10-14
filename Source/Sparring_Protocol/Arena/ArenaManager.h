#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Arena/ArenaGrid.h"
#include "ArenaManager.generated.h"

class UArenaRulesConfig;
class UStaticMeshComponent;
class AArenaSpawnPoint;

/**
 * Owns the arena grid. Place one in the level at the CENTER of the arena,
 * on the floor surface, with no rotation and scale 1.
 * The floor is generated from the grid size so both always match.
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

	const FArenaGrid& GetGrid() const { return Grid; }

	UFUNCTION(BlueprintPure, Category = "Arena")
	FVector CellToWorld(FIntVector Cell) const { return Grid.CellToWorld(Cell); }

	UFUNCTION(BlueprintPure, Category = "Arena")
	FIntVector WorldToCell(FVector WorldLocation) const { return Grid.WorldToCell(WorldLocation); }

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

	FArenaGrid Grid;
};
