#include "UI/SP_PauseScreen.h"

#include "Kismet/GameplayStatics.h"
#include "UI/SP_MenuButton.h"
#include "UI/SP_SettingsScreen.h"

void USP_PauseScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!SettingsScreenClass)
	{
		SettingsScreenClass = USP_SettingsScreen::StaticClass();
	}
}

void USP_PauseScreen::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (ResumeButton) { ResumeButton->SetLabel(NSLOCTEXT("SPMenu", "Resume", "RESUME")); }
	if (SettingsButton) { SettingsButton->SetLabel(NSLOCTEXT("SPMenu", "Settings", "SETTINGS")); }
	if (QuitButton) { QuitButton->SetLabel(NSLOCTEXT("SPMenu", "QuitToMenu", "QUIT TO MAIN MENU")); }
}

void USP_PauseScreen::NativeConstruct()
{
	Super::NativeConstruct();

	ResumeButton->OnPressed.AddUniqueDynamic(this, &USP_PauseScreen::HandleResume);
	SettingsButton->OnPressed.AddUniqueDynamic(this, &USP_PauseScreen::HandleSettings);
	QuitButton->OnPressed.AddUniqueDynamic(this, &USP_PauseScreen::HandleQuit);
}

UWidget* USP_PauseScreen::GetDefaultFocusWidget() const
{
	return ResumeButton;
}

void USP_PauseScreen::ShowPause()
{
	if (IsInViewport())
	{
		return;
	}

	AddToViewport(50);
	UGameplayStatics::SetGamePaused(this, true);

	// Game AND UI: the pause key (Trigger when Paused) can still close the menu.
	ActivateMenuInput(true);
	FocusDefault();
}

void USP_PauseScreen::HidePause()
{
	if (!IsInViewport())
	{
		return;
	}

	CloseChildScreen();
	RemoveFromParent();
	UGameplayStatics::SetGamePaused(this, false);
	ActivateGameInput();
}

bool USP_PauseScreen::HandleBack()
{
	HidePause();
	return true;
}

void USP_PauseScreen::HandleResume()
{
	HidePause();
}

void USP_PauseScreen::HandleSettings()
{
	OpenChildScreen(SettingsScreenClass, 60);
}

void USP_PauseScreen::HandleQuit()
{
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}
