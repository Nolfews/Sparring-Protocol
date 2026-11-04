#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "Arena/ArenaTypes.h"
#include "PlacementComponent.generated.h"

class AArenaManager;
class APlacementGhost;
class UPlaceableDefinition;
class UInputComponent;
class UMaterialInterface;

/** Fired whenever the validity of the previewed placement changes (for UI). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlacementPreviewChanged, EPlacementFailReason, Reason);

/**
 * Player side of the placement system. Add it to any actor (the ArenaManager for now,
 * the player pawn or controller later). It opens when the manager starts a build phase:
 * it follows the camera aim, snaps to the grid, shows a ghost, and places on confirm.
 * It talks to the local player 0 and pushes its own input component, so it needs no
 * input assets.
 */
UCLASS(ClassGroup = (Arena), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API UPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlacementComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void ConfirmPlacement();

	/** Toggles the ghost on and off so the player can look around freely. */
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void ToggleGhost();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void RotateGhost();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void CycleDefinition();

	/** Ends the build phase (the player is done building). */
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void FinishBuilding();

	UFUNCTION(BlueprintPure, Category = "Placement")
	bool IsPlacementActive() const { return bPlacementActive; }

	/** Short human readable text for a refusal reason. */
	UFUNCTION(BlueprintPure, Category = "Placement")
	static FText GetFailReasonText(EPlacementFailReason Reason);

	/** Elements the player can place, cycled with CycleKey. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
	TArray<TObjectPtr<UPlaceableDefinition>> AvailableDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement|Input")
	FKey ConfirmKey = EKeys::LeftMouseButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement|Input")
	FKey ToggleGhostKey = EKeys::RightMouseButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement|Input")
	FKey RotateKey = EKeys::R;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement|Input")
	FKey CycleKey = EKeys::Q;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement|Input")
	FKey FinishKey = EKeys::Enter;

	/** Maximum distance of the aiming trace, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "100"))
	float MaxAimDistance = 5000.f;

	/** Optional translucent materials for the ghost mesh. Without them only the debug box is drawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement|Visuals")
	TObjectPtr<UMaterialInterface> GhostValidMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement|Visuals")
	TObjectPtr<UMaterialInterface> GhostInvalidMaterial;

	/** Green / red wire box around the target cells. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement|Debug")
	bool bDrawDebugGhostBox = true;

	/** On-screen status and key help (debug HUD, to be replaced by the real UI). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement|Debug")
	bool bShowOnScreenHelp = true;

	UPROPERTY(BlueprintAssignable, Category = "Placement|Events")
	FOnPlacementPreviewChanged OnPreviewChanged;

private:
	UFUNCTION()
	void HandleBuildPhaseStarted(int32 PlayerSlots);

	UFUNCTION()
	void HandleBuildPhaseFinished();

	AArenaManager* ResolveManager() const;
	UPlaceableDefinition* GetCurrentDefinition() const;
	int32 GetEffectiveTurns(const UPlaceableDefinition* Definition) const;

	void EnterPlacementMode();
	void ExitPlacementMode();
	void UpdatePreview();
	void UpdateHud(const UPlaceableDefinition* Definition, const FString& Status, const FColor& StatusColor) const;

	UPROPERTY(Transient)
	TObjectPtr<AArenaManager> Manager;

	UPROPERTY(Transient)
	TObjectPtr<APlacementGhost> Ghost;

	UPROPERTY(Transient)
	TObjectPtr<UInputComponent> PlacementInput;

	bool bPlacementActive = false;
	bool bGhostEnabled = true;
	bool bHasAimCell = false;
	FIntVector LastAimCell = FIntVector::ZeroValue;
	int32 CurrentDefinitionIndex = 0;
	int32 QuarterTurns = 0;
	EPlacementFailReason LastReason = EPlacementFailReason::None;
};
