#pragma once

#include "CoreMinimal.h"
#include "UI/SP_MenuScreen.h"
#include "SP_MainMenuScreen.generated.h"

class UAkAudioEvent;
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

	/** Wwise event started when the menu opens (menu music). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Menu|Music")
	TSoftObjectPtr<UAkAudioEvent> MenuMusicEvent = TSoftObjectPtr<UAkAudioEvent>(FSoftObjectPath(TEXT("/Game/WwiseAudio/Events/Default_Work_Unit/Play_MX_Menu.Play_MX_Menu")));

	/** Wwise event that stops the menu music (with its fade out) when leaving the menu. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Menu|Music")
	TSoftObjectPtr<UAkAudioEvent> StopMenuMusicEvent = TSoftObjectPtr<UAkAudioEvent>(FSoftObjectPath(TEXT("/Game/WwiseAudio/Events/Default_Work_Unit/Stop_MX_Menu.Stop_MX_Menu")));

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
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

	void PlayMenuMusic();
	void StopMenuMusic();
	static void PostMenuEvent(const TSoftObjectPtr<UAkAudioEvent>& Event);

	bool bMusicPlaying = false;
};
