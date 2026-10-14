#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Arena/ArenaTypes.h"
#include "ArenaSpawnPoint.generated.h"

class UArrowComponent;

/** Spawn point for one side of the duel. Place one per side in the level. */
UCLASS()
class SPARRING_PROTOCOL_API AArenaSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AArenaSpawnPoint();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintPure, Category = "Arena")
	EPlacementOwner GetSpawnOwner() const { return SpawnOwner; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
	EPlacementOwner SpawnOwner = EPlacementOwner::Player;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UArrowComponent> Arrow;
};
