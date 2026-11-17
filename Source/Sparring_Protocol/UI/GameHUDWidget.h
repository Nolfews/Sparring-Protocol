#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "Arena/ArenaTypes.h"
#include "Game/GameFlowManager.h"
#include "GameHUDWidget.generated.h"

class UWidget;
class UProgressBar;
class UTextBlock;
class UImage;
class AArenaManager;
class APlaceableElement;
class UPlacementComponent;
class UPlaceableDefinition;
class USP_HealthComponent;
class USP_DashComponent;

/**
 * Base class of the in-game HUD. All the logic lives here: it finds the game flow, the arena and the
 * player on its own, listens to them and updates the widgets.
 *
 * The Blueprint child (WBP_GameHUD) only designs the layout. Any widget whose name and type match one
 * of the BindWidgetOptional properties below is driven automatically; widgets that are missing are
 * simply skipped, so the HUD can be built step by step.
 */
UCLASS()
class SPARRING_PROTOCOL_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Fills the dash segments: the first AvailableCharges are full, the next one shows RechargeProgress (0..1),
	 * the rest are empty. Segments beyond MaxCharges are hidden. To be called by whatever owns the dash charges.
	 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetDashState(int32 AvailableCharges, int32 MaxCharges, float RechargeProgress);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ---- Containers shown or hidden depending on the step of the run -------------------------

	/** Shown during every combat (tutorial and rounds): player bars, dashes, round number. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> CombatPanel;

	/** Shown during a combat while the enemy exists (health bar at the top). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> EnemyPanel;

	/** Shown during every build phase. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> BuildPanel;

	/** Shown during the two tutorial steps (combat and build). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> TutorialPanel;

	/** Shown in combat and in build (it also helps to aim the placement). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Crosshair;

	// ---- Combat ------------------------------------------------------------------------------

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> PlayerHealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerHealthText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> DashSegment0;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> DashSegment1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> DashSegment2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> EnemyHealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EnemyHealthText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RoundText;

	// ---- Build -------------------------------------------------------------------------------

	/** Icon of the selected element (rotated with the R key). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ShapeIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ShapeNameText;

	/** Footprint of the selected element, for example "2 x 1". */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ShapeSizeText;

	/** "placed / maximum". */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SlotsText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> SlotsBar;

	/** Why the current placement is refused (empty when it is valid). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText;

	/** Key labels, read from the placement component so they always match the real keys. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlaceKeyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RotateKeyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CycleKeyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FinishKeyText;

	// ---- Tutorial ----------------------------------------------------------------------------

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TutorialTitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TutorialBodyText;

	// ---- Texts, editable in the Blueprint child ------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Texts")
	FText RoundFormat = NSLOCTEXT("SPHud", "RoundFormat", "ROUND {0}");

	/** Shown in place of the round number during the combat tutorial. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Texts")
	FText TutorialRoundLabel = NSLOCTEXT("SPHud", "TutorialRound", "TUTORIAL");

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Texts")
	FText SlotsFormat = NSLOCTEXT("SPHud", "SlotsFormat", "{0} / {1}");

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Texts")
	FText CombatTutorialTitle = NSLOCTEXT("SPHud", "CombatTutorialTitle", "TUTORIAL - COMBAT");

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Texts", meta = (MultiLine = true))
	FText CombatTutorialBody = NSLOCTEXT("SPHud", "CombatTutorialBody", "Move, jump, dash and shoot. \nThe enemy does not fight back. Execute him.");

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Texts")
	FText BuildTutorialTitle = NSLOCTEXT("SPHud", "BuildTutorialTitle", "TUTORIAL - BUILDING");

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Texts", meta = (MultiLine = true))
	FText BuildTutorialBody = NSLOCTEXT("SPHud", "BuildTutorialBody", "Place elements to shape the arena. You get more slots after every round won.\nElements wear out over the rounds and break under fire.");

	/** Replaces the default label of a key (for example Enter -> ENTRÉE, Left Mouse Button -> CLIC GAUCHE). */
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Texts")
	TMap<FKey, FText> KeyLabelOverrides;

private:
	UFUNCTION()
	void HandleFlowStateChanged(EGameFlowState NewState, EGameFlowState PreviousState);

	UFUNCTION()
	void HandleRoundStarted(int32 RoundNumber);

	UFUNCTION()
	void HandleEnemySpawned(APawn* Enemy);

	UFUNCTION()
	void HandleBuildPhaseStarted(int32 PlayerSlots);

	UFUNCTION()
	void HandleBuildPhaseFinished();

	UFUNCTION()
	void HandleElementPlaced(APlaceableElement* Element);

	UFUNCTION()
	void HandleElementRemoved(APlaceableElement* Element);

	UFUNCTION()
	void HandlePlayerHealthChanged(USP_HealthComponent* ChangedComponent, float NewHealth, float Delta);

	UFUNCTION()
	void HandleEnemyHealthChanged(USP_HealthComponent* ChangedComponent, float NewHealth, float Delta);

	UFUNCTION()
	void HandleSelectionChanged(UPlaceableDefinition* SelectedDefinition, int32 SelectedQuarterTurns);

	UFUNCTION()
	void HandlePreviewChanged(EPlacementFailReason Reason);

	void TryBindPlayer();
	void BindEnemy(APawn* Enemy);

	void RefreshAll();
	void ApplyState(EGameFlowState State);
	void RefreshPlayerHealth();
	void RefreshEnemyHealth();
	void RefreshRound();
	void RefreshSlots();
	void RefreshSelection();
	void RefreshKeyHints();
	void RefreshTutorial(EGameFlowState State);

	FText GetKeyLabel(const FKey& Key) const;

	UPROPERTY(Transient)
	TObjectPtr<AGameFlowManager> GameFlow;

	UPROPERTY(Transient)
	TObjectPtr<AArenaManager> Arena;

	UPROPERTY(Transient)
	TObjectPtr<UPlacementComponent> Placement;

	TWeakObjectPtr<USP_HealthComponent> PlayerHealth;
	TWeakObjectPtr<USP_DashComponent> PlayerDash;

	// Last values sent to the dash segments, to update them only when something changed.
	int32 LastDashCharges = -1;
	int32 LastDashMax = -1;
	float LastDashProgress = -1.f;
	TWeakObjectPtr<USP_HealthComponent> EnemyHealth;
};
