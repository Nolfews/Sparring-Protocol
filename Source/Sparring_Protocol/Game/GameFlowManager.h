#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "GameFlowManager.generated.h"

class AArenaManager;
class UPlaceableDefinition;
class UInputComponent;

/** Where the run currently is. */
UENUM(BlueprintType)
enum class EGameFlowState : uint8
{
	Idle,
	/** Empty arena, passive enemy: the player learns the controls. */
	TutorialCombat,
	/** First build phase of a run, with guidance. */
	TutorialBuild,
	/** Build phase between rounds. */
	Build,
	RoundCombat,
	GameOver
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameFlowStateChanged, EGameFlowState, NewState, EGameFlowState, PreviousState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStarted, int32, RoundNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoundEnded, int32, RoundNumber, bool, bPlayerWon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunEnded, int32, RoundsWon);

/**
 * Drives the run: tutorial combat -> tutorial build -> round 1 -> build -> round 2 -> build ...
 * until the player is defeated.
 *
 * Combat is not implemented here: whoever owns combat calls NotifyRoundWon() and
 * NotifyPlayerDefeated(), and listens to OnRoundStarted / OnStateChanged
 * (for example to scale the enemy stats or keep it passive during the combat tutorial).
 * Place one in the level. It can be moved into the GameMode later without API changes.
 */
UCLASS()
class SPARRING_PROTOCOL_API AGameFlowManager : public AActor
{
	GENERATED_BODY()

public:
	AGameFlowManager();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ---- Contract for combat / UI --------------------------------------------

	/** Starts a new run: clears the arena and enters the first step. */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void StartRun();

	/** Combat side: the player killed the enemy. Opens the next build phase. */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void NotifyRoundWon();

	/** Combat side: the player died. Ends the run. */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void NotifyPlayerDefeated();

	/** Ends the combat tutorial and opens the build tutorial. */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void CompleteCombatTutorial();

	UFUNCTION(BlueprintPure, Category = "GameFlow")
	EGameFlowState GetState() const { return State; }

	/** Consecutive rounds won in the current run (the score). */
	UFUNCTION(BlueprintPure, Category = "GameFlow")
	int32 GetRoundsWon() const { return RoundsWon; }

	/** Number of the round being fought (0 outside combat rounds). */
	UFUNCTION(BlueprintPure, Category = "GameFlow")
	int32 GetCurrentRound() const { return CurrentRound; }

	/** True while the enemy must stay passive. */
	UFUNCTION(BlueprintPure, Category = "GameFlow")
	bool IsTutorialCombat() const { return State == EGameFlowState::TutorialCombat; }

	UFUNCTION(BlueprintPure, Category = "GameFlow")
	bool IsCombatActive() const { return State == EGameFlowState::TutorialCombat || State == EGameFlowState::RoundCombat; }

	// ---- Events --------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Events")
	FOnGameFlowStateChanged OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Events")
	FOnRoundStarted OnRoundStarted;

	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Events")
	FOnRoundEnded OnRoundEnded;

	UPROPERTY(BlueprintAssignable, Category = "GameFlow|Events")
	FOnRunEnded OnRunEnded;

	// ---- Debug (play in editor: keys, or select this actor and use the buttons) ----

	/** Advances the flow: tutorial combat -> build, finish building, win the round, restart after game over. */
	UFUNCTION(CallInEditor, Category = "Debug|Flow")
	void Debug_NextStep();

	UFUNCTION(CallInEditor, Category = "Debug|Flow")
	void Debug_LoseRound();

	UFUNCTION(CallInEditor, Category = "Debug|Flow")
	void Debug_RestartRun();

	/** Makes both tutorials play again on the next run. */
	UFUNCTION(CallInEditor, Category = "Debug|Flow")
	void Debug_ResetTutorials();

protected:
	/** Arena to drive. Found automatically if left empty. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "GameFlow")
	TObjectPtr<AArenaManager> ArenaManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameFlow")
	bool bAutoStartRun = true;

	/** Dev override: never play the tutorials. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameFlow")
	bool bSkipTutorials = false;

	/** The enemy places random elements from this list at each build phase (not during the tutorial). Empty = it does not build. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameFlow|Enemy")
	TArray<TObjectPtr<UPlaceableDefinition>> EnemyDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug|Flow")
	FKey NextStepKey = EKeys::F6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug|Flow")
	FKey LoseRoundKey = EKeys::F7;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug|Flow")
	FKey RestartKey = EKeys::F9;

	/** Set once the tutorial was played. TODO: persist in a SaveGame. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameFlow")
	bool bCombatTutorialCompleted = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameFlow")
	bool bBuildTutorialCompleted = false;

private:
	UFUNCTION()
	void HandleBuildPhaseFinished();

	void SetState(EGameFlowState NewState);
	void BeginBuildStep();
	void StartCombatRound();
	void PlaceEnemyElements();
	void BindDebugInput();
	void RefreshHud() const;
	bool CanRunDebugAction() const;

	UPROPERTY(VisibleAnywhere, Category = "GameFlow")
	EGameFlowState State = EGameFlowState::Idle;

	UPROPERTY(VisibleAnywhere, Category = "GameFlow")
	int32 RoundsWon = 0;

	UPROPERTY(VisibleAnywhere, Category = "GameFlow")
	int32 CurrentRound = 0;

	UPROPERTY(Transient)
	TObjectPtr<UInputComponent> DebugInput;
};
