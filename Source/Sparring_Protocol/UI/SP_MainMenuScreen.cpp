#include "UI/SP_MainMenuScreen.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/SP_LeaderboardScreen.h"
#include "UI/SP_MenuButton.h"
#include "UI/SP_SettingsScreen.h"

void USP_MainMenuScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!LeaderboardScreenClass)
	{
		LeaderboardScreenClass = USP_LeaderboardScreen::StaticClass();
	}
	if (!SettingsScreenClass)
	{
		SettingsScreenClass = USP_SettingsScreen::StaticClass();
	}
}

void USP_MainMenuScreen::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (PlayButton) { PlayButton->SetLabel(NSLOCTEXT("SPMenu", "Play", "PLAY")); }
	if (LeaderboardButton) { LeaderboardButton->SetLabel(NSLOCTEXT("SPMenu", "Leaderboard", "LEADERBOARD")); }
	if (SettingsButton) { SettingsButton->SetLabel(NSLOCTEXT("SPMenu", "Settings", "SETTINGS")); }
	if (QuitButton) { QuitButton->SetLabel(NSLOCTEXT("SPMenu", "Quit", "QUIT")); }
}

void USP_MainMenuScreen::NativeConstruct()
{
	Super::NativeConstruct();

	PlayButton->OnPressed.AddUniqueDynamic(this, &USP_MainMenuScreen::HandlePlay);
	LeaderboardButton->OnPressed.AddUniqueDynamic(this, &USP_MainMenuScreen::HandleLeaderboard);
	SettingsButton->OnPressed.AddUniqueDynamic(this, &USP_MainMenuScreen::HandleSettings);
	QuitButton->OnPressed.AddUniqueDynamic(this, &USP_MainMenuScreen::HandleQuit);

	ActivateMenuInput(false);
	FocusDefault();
}

UWidget* USP_MainMenuScreen::GetDefaultFocusWidget() const
{
	return PlayButton;
}

void USP_MainMenuScreen::HandlePlay()
{
	// The input mode survives the level change: give the controls back to the game first.
	ActivateGameInput();
	UGameplayStatics::OpenLevel(this, ArenaLevelName);
}

void USP_MainMenuScreen::HandleLeaderboard()
{
	OpenChildScreen(LeaderboardScreenClass);
}

void USP_MainMenuScreen::HandleSettings()
{
	OpenChildScreen(SettingsScreenClass);
}

void USP_MainMenuScreen::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
