#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Game/GameFlowManager.h"
#include "SP_GameFlowLinkComponent.generated.h"

class USP_AbilityComponent;

UENUM(BlueprintType)
enum class ESPCombatantRole : uint8
{
	Player,
	Enemy
};

/**
 * Connects a combat character to the AGameFlowManager of the level, without either side
 * knowing the other:
 *  - Player: revived and healed at each combat, some abilities (shoot) disabled during the
 *    build phases, NotifyPlayerDefeated() on death.
 *  - Enemy: NotifyRoundWon() on death (CompleteCombatTutorial() during the combat tutorial).
 *
 * Add it to the player / enemy Blueprint that plays in the arena. Without a GameFlowManager
 * in the level (test maps), it does nothing.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API USP_GameFlowLinkComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USP_GameFlowLinkComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameFlow")
	ESPCombatantRole Role = ESPCombatantRole::Player;

	/** Player only: abilities turned off during the build phases (the left click places elements). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameFlow", meta = (EditCondition = "Role == ESPCombatantRole::Player"))
	TArray<TSubclassOf<USP_AbilityComponent>> AbilitiesDisabledWhileBuilding;

	/** Time between the death and the notification to the game flow (death animation...). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameFlow", meta = (ClampMin = "0", Units = "s"))
	float DeathNotifyDelay = 0.5f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleStateChanged(EGameFlowState NewState, EGameFlowState PreviousState);

	UFUNCTION()
	void HandleOwnerDeath(AActor* Killer);

	void ApplyPlayerState(EGameFlowState State);

	UPROPERTY(Transient)
	TObjectPtr<AGameFlowManager> GameFlow;
};
