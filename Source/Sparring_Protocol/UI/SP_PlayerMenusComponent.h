#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SP_PlayerMenusComponent.generated.h"

class AGameFlowManager;
class APlayerController;
class UEnhancedInputComponent;
class UInputAction;
class USP_DeathScreen;
class USP_PauseScreen;

/**
 * In-game menus of the player (created by ASP_PlayerCharacter, nothing to add in the Blueprint):
 *  - the pause key (IA_Pause) opens / closes WBP_PauseMenu
 *  - when the GameFlowManager ends the run, WBP_DeathScreen is shown with the score
 * The asset paths below are the defaults; change them in the details panel if an asset moves.
 */
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API USP_PlayerMenusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USP_PlayerMenusComponent();

	/** Called by the player character when its input is set up. */
	void BindInput(UEnhancedInputComponent* EnhancedInput);

	UFUNCTION(BlueprintCallable, Category = "Menus")
	void TogglePause();

	UFUNCTION(BlueprintCallable, Category = "Menus")
	void ShowDeathScreen(int32 RoundsWon);

	/** Must have "Trigger when Paused" checked, so it can also close the menu. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menus")
	TSoftObjectPtr<UInputAction> PauseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menus")
	TSoftClassPtr<USP_PauseScreen> PauseScreenClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menus")
	TSoftClassPtr<USP_DeathScreen> DeathScreenClass;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleRunEnded(int32 RoundsWon);

	APlayerController* GetPlayerController() const;

	UPROPERTY(Transient)
	TObjectPtr<AGameFlowManager> GameFlow;

	UPROPERTY(Transient)
	TObjectPtr<USP_PauseScreen> PauseScreen;

	UPROPERTY(Transient)
	TObjectPtr<USP_DeathScreen> DeathScreen;
};
