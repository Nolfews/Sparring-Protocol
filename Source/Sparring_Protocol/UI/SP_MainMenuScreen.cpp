#include "UI/SP_MainMenuScreen.h"

#include "AkAudioEvent.h"
#include "AkGameplayStatics.h"
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
	PlayMenuMusic();
}

void USP_MainMenuScreen::NativeDestruct()
{
	StopMenuMusic();
	Super::NativeDestruct();
}

UWidget* USP_MainMenuScreen::GetDefaultFocusWidget() const
{
	return PlayButton;
}

void USP_MainMenuScreen::HandlePlay()
{
	// The input mode survives the level change: give the controls back to the game first.
	ActivateGameInput();
	StopMenuMusic();
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

void USP_MainMenuScreen::PlayMenuMusic()
{
	if (!bMusicPlaying)
	{
		PostMenuEvent(MenuMusicEvent);
		bMusicPlaying = true;
	}
}

void USP_MainMenuScreen::StopMenuMusic()
{
	if (bMusicPlaying)
	{
		PostMenuEvent(StopMenuMusicEvent);
		bMusicPlaying = false;
	}
}

void USP_MainMenuScreen::PostMenuEvent(const TSoftObjectPtr<UAkAudioEvent>& Event)
{
	// No actor: the music is played as an ambient (2D) sound, like the combat music.
	if (UAkAudioEvent* LoadedEvent = Event.LoadSynchronous())
	{
		UAkGameplayStatics::PostEvent(LoadedEvent, nullptr, 0, FOnAkPostEventCallback(), false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Menu music event not found: %s"), *Event.ToString());
	}
}

void USP_MainMenuScreen::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
