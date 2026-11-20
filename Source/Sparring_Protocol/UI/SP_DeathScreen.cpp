#include "UI/SP_DeathScreen.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Score/SP_LeaderboardSubsystem.h"
#include "UI/SP_MenuButton.h"
#include "UI/SP_ScoreList.h"
#include "UI/SP_UIStyle.h"

namespace
{
	FText DeathScreenTwoDigits(int32 Value)
	{
		return FText::FromString(FString::Printf(TEXT("%02d"), Value));
	}

	bool DeathScreenIsAnyOf(const FKey& Key, std::initializer_list<FKey> Keys)
	{
		for (const FKey& Candidate : Keys)
		{
			if (Key == Candidate)
			{
				return true;
			}
		}
		return false;
	}
}

void USP_DeathScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Hall of Fame panel, added in code on the right of the root canvas.
	if (UCanvasPanel* Root = Cast<UCanvasPanel>(GetRootWidget()))
	{
		HallOfFame = WidgetTree->ConstructWidget<USP_ScoreList>(USP_ScoreList::StaticClass());
		HallOfFame->MaxRows = HallOfFameRows;

		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(HallOfFame);
		PanelSlot->SetAnchors(FAnchors(1.f, 0.f));
		PanelSlot->SetAlignment(FVector2D(1.f, 0.f));
		PanelSlot->SetPosition(HallOfFameOffset);
		PanelSlot->SetAutoSize(true);
	}

	RetryButton->OnPressed.AddUniqueDynamic(this, &USP_DeathScreen::HandleRetry);
	MainMenuButton->OnPressed.AddUniqueDynamic(this, &USP_DeathScreen::HandleMainMenu);
}

void USP_DeathScreen::NativeConstruct()
{
	Super::NativeConstruct();

	RetryButton->SetLabel(NSLOCTEXT("SPMenu", "Retry", "RETRY"));
	MainMenuButton->SetLabel(NSLOCTEXT("SPMenu", "MainMenu", "MAIN MENU"));

	bConstructed = true;
	RefreshAll();

	if (Flicker)
	{
		PlayAnimation(Flicker, 0.f, 0);
	}
}

void USP_DeathScreen::ShowResults(int32 InRoundsWon, int32 InRoundReached)
{
	RoundsWon = FMath::Max(0, InRoundsWon);
	RoundReached = FMath::Max(1, InRoundReached);

	const USP_LeaderboardSubsystem* Leaderboard = GetLeaderboard();
	bIsNewBest = Leaderboard && RoundsWon > 0 && RoundsWon > Leaderboard->GetBestRounds();
	bEnteringName = Leaderboard && Leaderboard->QualifiesForBoard(RoundsWon);
	PlayerName = Leaderboard ? USP_LeaderboardSubsystem::SanitizeName(Leaderboard->GetLastName()) : FString(TEXT("AAA"));
	ActiveLetter = 0;
	SavedIndex = INDEX_NONE;

	if (bConstructed)
	{
		RefreshAll();
		FocusDefault();
	}
}

USP_LeaderboardSubsystem* USP_DeathScreen::GetLeaderboard() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<USP_LeaderboardSubsystem>() : nullptr;
}

UWidget* USP_DeathScreen::GetDefaultFocusWidget() const
{
	// While typing the name, the screen keeps the focus to receive the arrows.
	return bEnteringName ? nullptr : RetryButton.Get();
}

void USP_DeathScreen::RefreshAll()
{
	KnockoutText->SetText(FText::Format(NSLOCTEXT("SPMenu", "KnockedOut", "KNOCKED OUT IN ROUND {0}"), DeathScreenTwoDigits(RoundReached)));
	ScoreText->SetText(DeathScreenTwoDigits(RoundsWon));
	NewBestBadge->SetVisibility(bIsNewBest ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	const bool bShowName = bEnteringName || SavedIndex != INDEX_NONE;
	const ESlateVisibility NameVisibility = bShowName ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	NameLabel->SetVisibility(NameVisibility);
	NameEntry->SetVisibility(NameVisibility);
	NameLabel->SetText(bEnteringName
		? NSLOCTEXT("SPMenu", "EnterName", "ENTER YOUR NAME  ·  ARROWS + ENTER")
		: NSLOCTEXT("SPMenu", "NameSaved", "SAVED IN THE HALL OF FAME"));

	// Mouse hover must not steal the focus from the name entry.
	RetryButton->bFocusOnHover = !bEnteringName;
	MainMenuButton->bFocusOnHover = !bEnteringName;

	RefreshLetters();
	RefreshHallOfFame();
}

void USP_DeathScreen::RefreshLetters()
{
	UTextBlock* Letters[] = { Letter1, Letter2, Letter3 };
	for (int32 Index = 0; Index < 3; ++Index)
	{
		if (!Letters[Index])
		{
			continue;
		}
		const TCHAR Character = PlayerName.IsValidIndex(Index) ? PlayerName[Index] : TEXT('A');
		Letters[Index]->SetText(FText::FromString(FString::Chr(Character)));

		const bool bActive = bEnteringName && Index == ActiveLetter;
		Letters[Index]->SetColorAndOpacity(FSlateColor(bActive ? SPUIStyle::Green() : SPUIStyle::TextLight()));
	}
}

void USP_DeathScreen::RefreshHallOfFame()
{
	const USP_LeaderboardSubsystem* Leaderboard = GetLeaderboard();
	if (!HallOfFame || !Leaderboard)
	{
		return;
	}

	TArray<FSPLeaderboardEntry> Entries = Leaderboard->GetEntries();
	int32 Highlight = SavedIndex;

	// Before saving: preview where the score will land, with the name being typed.
	if (bEnteringName)
	{
		FSPLeaderboardEntry Preview;
		Preview.Name = PlayerName;
		Preview.Rounds = RoundsWon;
		Highlight = Leaderboard->GetInsertIndex(RoundsWon);
		Entries.Insert(Preview, Highlight);
	}

	HallOfFame->SetEntries(Entries, Highlight);
}

FReply USP_DeathScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bEnteringName)
	{
		const FKey Key = InKeyEvent.GetKey();

		if (DeathScreenIsAnyOf(Key, { EKeys::Up, EKeys::Gamepad_DPad_Up, EKeys::Gamepad_LeftStick_Up }))
		{
			CycleLetter(+1);
			return FReply::Handled();
		}
		if (DeathScreenIsAnyOf(Key, { EKeys::Down, EKeys::Gamepad_DPad_Down, EKeys::Gamepad_LeftStick_Down }))
		{
			CycleLetter(-1);
			return FReply::Handled();
		}
		if (DeathScreenIsAnyOf(Key, { EKeys::Left, EKeys::Gamepad_DPad_Left, EKeys::Gamepad_LeftStick_Left, EKeys::BackSpace }))
		{
			MoveCursor(-1);
			return FReply::Handled();
		}
		if (DeathScreenIsAnyOf(Key, { EKeys::Right, EKeys::Gamepad_DPad_Right, EKeys::Gamepad_LeftStick_Right }))
		{
			MoveCursor(+1);
			return FReply::Handled();
		}
		if (IsAcceptKey(Key) || IsBackKey(Key))
		{
			ConfirmName();
			return FReply::Handled();
		}

		// Typing a letter directly on the keyboard.
		const FString KeyName = Key.GetFName().ToString();
		if (KeyName.Len() == 1 && KeyName[0] >= TEXT('A') && KeyName[0] <= TEXT('Z'))
		{
			TypeLetter(KeyName[0]);
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void USP_DeathScreen::CycleLetter(int32 Direction)
{
	if (!PlayerName.IsValidIndex(ActiveLetter))
	{
		return;
	}
	const int32 Current = PlayerName[ActiveLetter] - TEXT('A');
	const int32 Next = (Current + Direction + 26) % 26;
	PlayerName[ActiveLetter] = static_cast<TCHAR>(TEXT('A') + Next);

	RefreshLetters();
	RefreshHallOfFame();
}

void USP_DeathScreen::MoveCursor(int32 Direction)
{
	ActiveLetter = FMath::Clamp(ActiveLetter + Direction, 0, 2);
	RefreshLetters();
}

void USP_DeathScreen::TypeLetter(TCHAR Letter)
{
	if (PlayerName.IsValidIndex(ActiveLetter))
	{
		PlayerName[ActiveLetter] = Letter;
	}
	ActiveLetter = FMath::Min(ActiveLetter + 1, 2);
	RefreshLetters();
	RefreshHallOfFame();
}

void USP_DeathScreen::ConfirmName()
{
	if (!bEnteringName)
	{
		return;
	}

	if (USP_LeaderboardSubsystem* Leaderboard = GetLeaderboard())
	{
		SavedIndex = Leaderboard->AddEntry(PlayerName, RoundsWon);
	}
	bEnteringName = false;

	RefreshAll();
	FocusDefault();
}

void USP_DeathScreen::HandleRetry()
{
	ConfirmName(); // Leaving with the mouse still saves the score.
	ActivateGameInput();
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}

void USP_DeathScreen::HandleMainMenu()
{
	ConfirmName();
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}

void USP_DeathScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Broken neon: the O goes off for a short random time, then back on for a longer one.
	if (Title_O && bRandomFlickerOnO)
	{
		FlickerTimer -= InDeltaTime;
		if (FlickerTimer <= 0.f)
		{
			bOFlickerOff = !bOFlickerOff;
			FlickerTimer = bOFlickerOff ? FMath::FRandRange(0.04f, 0.4f) : FMath::FRandRange(0.15f, 2.2f);
			Title_O->SetRenderOpacity(bOFlickerOff ? 0.15f : 1.f);
		}
	}
}
