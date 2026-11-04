#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Arena/ArenaTypes.h"
#include "PlaceableElement.generated.h"

class UPlaceableDefinition;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Base class of everything the arena can hold (walls, blocks, ramps...).
 * Child Blueprints only set the mesh, materials and effects. No gameplay logic.
 * The actor pivot is the CENTER of its footprint box.
 *
 * Lifetime: the definition gives a number of rounds (0 = permanent). The ArenaManager
 * ages every element once per round. The material gets a "CrackAmount" scalar going
 * from 0 (intact) towards 1 (about to break) and an "OwnerTint" vector.
 */
UCLASS()
class SPARRING_PROTOCOL_API APlaceableElement : public AActor
{
	GENERATED_BODY()

public:
	APlaceableElement();

	virtual void Tick(float DeltaSeconds) override;

	/** Called by the ArenaManager right after spawning, before BeginPlay. */
	void Initialize(UPlaceableDefinition* InDefinition, EPlacementOwner InOwner, const TArray<FIntVector>& InCells, const FVector& InWorldBoxSize);

	UFUNCTION(BlueprintPure, Category = "Placeable")
	UPlaceableDefinition* GetDefinition() const { return Definition; }

	UFUNCTION(BlueprintPure, Category = "Placeable")
	EPlacementOwner GetPlacementOwner() const { return PlacementOwner; }

	const TArray<FIntVector>& GetOccupiedCells() const { return OccupiedCells; }

	/** Size in cm of the box this element fills (rotated footprint times cell size). Handy for break effects. */
	UFUNCTION(BlueprintPure, Category = "Placeable")
	FVector GetWorldBoxSize() const { return WorldBoxSize; }

	/** Color of whoever placed it, to tint effects like its debris. */
	UFUNCTION(BlueprintPure, Category = "Placeable")
	FLinearColor GetOwnerTint() const { return PlacementOwner == EPlacementOwner::Player ? PlayerTint : EnemyTint; }

	// ---- Lifetime ---------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Placeable|Lifetime")
	bool IsPermanent() const { return TotalRounds <= 0; }

	UFUNCTION(BlueprintPure, Category = "Placeable|Lifetime")
	int32 GetRoundsRemaining() const { return RoundsRemaining; }

	UFUNCTION(BlueprintPure, Category = "Placeable|Lifetime")
	int32 GetTotalRounds() const { return TotalRounds; }

	/** 0 = intact, 1 = broken. Drives the crack material parameter. */
	UFUNCTION(BlueprintPure, Category = "Placeable|Lifetime")
	float GetDamageFraction() const;

	/** Ages the element by one round. Returns true when it just ran out (the ArenaManager then removes it). */
	bool AgeOneRound();

	/** Called by the ArenaManager when the element leaves the arena: disables it and plays the break effect. */
	void Break();

protected:
	virtual void BeginPlay() override;

	/** Lets a Blueprint react to who placed it (extra visuals). The tint itself is already applied. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Placeable")
	void OnOwnerAssigned(EPlacementOwner NewOwner);

	/** Fired each time the element ages (for extra VFX or sound). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Placeable|Lifetime")
	void OnLifetimeChanged(int32 NewRoundsRemaining, int32 NewTotalRounds);

	/** Fired when the element breaks: spawn debris, particles and sound here (use "at location" nodes, this actor disappears soon). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Placeable|Lifetime")
	void OnBroken();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Scalar parameter of the material driven from 0 (intact) to 1 (broken). */
	UPROPERTY(EditDefaultsOnly, Category = "Placeable|Visuals")
	FName CrackParameterName = TEXT("CrackAmount");

	/** Vector parameter of the material set to the owner's color. */
	UPROPERTY(EditDefaultsOnly, Category = "Placeable|Visuals")
	FName OwnerTintParameterName = TEXT("OwnerTint");

	UPROPERTY(EditDefaultsOnly, Category = "Placeable|Visuals")
	FLinearColor PlayerTint = FLinearColor(0.35f, 0.65f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Placeable|Visuals")
	FLinearColor EnemyTint = FLinearColor(1.f, 0.4f, 0.3f);

	/** How fast the crack amount animates towards its new value (units per second). */
	UPROPERTY(EditDefaultsOnly, Category = "Placeable|Visuals", meta = (ClampMin = "0.1"))
	float CrackTransitionSpeed = 1.5f;

	/** Hide the mesh the moment the element breaks (turn off if your OnBroken animates it). */
	UPROPERTY(EditDefaultsOnly, Category = "Placeable|Lifetime")
	bool bHideMeshOnBreak = true;

	/** Time before the actor is destroyed after breaking. */
	UPROPERTY(EditDefaultsOnly, Category = "Placeable|Lifetime", meta = (ClampMin = "0.1"))
	float BreakLifeSpan = 1.5f;

private:
	void ApplyCrack(float Amount);
	void ApplyOwnerTint();

	UPROPERTY(Transient)
	TObjectPtr<UPlaceableDefinition> Definition;

	UPROPERTY(Transient)
	EPlacementOwner PlacementOwner = EPlacementOwner::Player;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	TArray<FIntVector> OccupiedCells;

	FVector WorldBoxSize = FVector(100.f);

	int32 RoundsRemaining = 0;
	int32 TotalRounds = 0;
	float CurrentCrack = 0.f;
	float TargetCrack = 0.f;
	bool bBroken = false;
};
