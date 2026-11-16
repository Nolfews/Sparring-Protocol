#pragma once

#include "CoreMinimal.h"
#include "UI/SP_MenuScreen.h"
#include "SP_MainMenuScreen.generated.h"

class USP_MenuButton;

/**
 * Parent class of WBP_MainMenu. The Blueprint keeps the layout; the four buttons
 * must be WBP_MenuButton named PlayButton, LeaderboardButton, SettingsButton, QuitButton.
 */
UCLASS(Abstract)
class SPARRING_PROTOCOL_API USP_MainMenuScreen : public USP_MenuScreen
{
	GENERATED_BODY()

public:
	/** Level loaded by PLAY. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Menu")
	FName ArenaLevelName = TEXT("L_Arena");

	/** Screen opened by LEADERBOARD (built in code by default). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Menu")
	TSubclassOf<USP_MenuScreen> LeaderboardScreenClass;

	/** Screen opened by SETTINGS (built in code by default). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Menu")
	TSubclassOf<USP_MenuScreen> SettingsScreenClass;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual UWidget* GetDefaultFocusWidget() const override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USP_MenuButton> PlayButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USP_MenuButton> LeaderboardButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USP_MenuButton> SettingsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USP_MenuButton> QuitButton;

private:
	UFUNCTION()
	void HandlePlay();

	UFUNCTION()
	void HandleLeaderboard();

	UFUNCTION()
	void HandleSettings();

	UFUNCTION()
	void HandleQuit();
};
