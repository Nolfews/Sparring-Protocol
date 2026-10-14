#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Arena/ArenaTypes.h"
#include "PlaceableElement.generated.h"

class UPlaceableDefinition;
class UStaticMeshComponent;

/**
 * Base class of everything the arena can hold (walls, blocks, ramps...).
 * Child Blueprints only set the mesh, materials and collision. No logic.
 * The actor pivot is the CENTER of its footprint box.
 */
UCLASS()
class SPARRING_PROTOCOL_API APlaceableElement : public AActor
{
	GENERATED_BODY()

public:
	APlaceableElement();

	/** Called by the ArenaManager right after spawning, before BeginPlay. */
	void Initialize(UPlaceableDefinition* InDefinition, EPlacementOwner InOwner, const TArray<FIntVector>& InCells);

	UFUNCTION(BlueprintPure, Category = "Placeable")
	UPlaceableDefinition* GetDefinition() const { return Definition; }

	UFUNCTION(BlueprintPure, Category = "Placeable")
	EPlacementOwner GetPlacementOwner() const { return PlacementOwner; }

	const TArray<FIntVector>& GetOccupiedCells() const { return OccupiedCells; }

protected:
	virtual void BeginPlay() override;

	/** Lets a Blueprint tint the mesh depending on who placed it. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Placeable")
	void OnOwnerAssigned(EPlacementOwner NewOwner);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPlaceableDefinition> Definition;

	UPROPERTY(Transient)
	EPlacementOwner PlacementOwner = EPlacementOwner::Player;

	TArray<FIntVector> OccupiedCells;
};
