#pragma once

#include "CoreMinimal.h"
#include "UI/SP_MenuScreen.h"
#include "SP_PauseScreen.generated.h"

class USP_MenuButton;

/**
 * Parent class of WBP_PauseMenu. Buttons: ResumeButton, SettingsButton, QuitButton (WBP_MenuButton).
 * Opened / closed by USP_PlayerMenusComponent (pause key). Escape or gamepad B also resumes.
 */
UCLASS(Abstract)
class SPARRING_PROTOCOL_API USP_PauseScreen : public USP_MenuScreen
{
	GENERATED_BODY()

public:
	/** Shows the menu and pauses the game. */
	UFUNCTION(BlueprintCallable, Category = "Pause")
	void ShowPause();

	/** Hides the menu and resumes the game. */
	UFUNCTION(BlueprintCallable, Category = "Pause")
	void HidePause();

	UFUNCTION(BlueprintPure, Category = "Pause")
	bool IsPauseShown() const { return IsInViewport(); }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pause")
	FName MainMenuLevelName = TEXT("L_MainMenu");

	/** Screen opened by SETTINGS (built in code by default). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pause")
	TSubclassOf<USP_MenuScreen> SettingsScreenClass;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual UWidget* GetDefaultFocusWidget() const override;
	virtual bool HandleBack() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USP_MenuButton> ResumeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USP_MenuButton> SettingsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USP_MenuButton> QuitButton;

private:
	UFUNCTION()
	void HandleResume();

	UFUNCTION()
	void HandleSettings();

	UFUNCTION()
	void HandleQuit();
};
