#pragma once

#include "CoreMinimal.h"
#include "UI/SP_MenuScreen.h"
#include "SP_DeathScreen.generated.h"

class UTextBlock;
class UWidgetAnimation;
class USP_LeaderboardSubsystem;
class USP_MenuButton;
class USP_ScoreList;

/**
 * Parent class of WBP_DeathScreen. Shown by USP_PlayerMenusComponent when the run ends.
 *  - score, round and NEW BEST badge
 *  - if the score enters the top 10: arcade name entry (3 letters)
 *      arrows / D-pad up-down = change the letter, left-right = move, or type the letters,
 *      Enter / gamepad A = save
 *  - Hall of Fame panel (top 5) added in code on the right, with the new score in green
 *  - RETRY reloads the level, MAIN MENU goes back to the menu
 *  - the "Flicker" animation loops, and the O of "LIGHTS OUT" flickers randomly
 */
UCLASS(Abstract)
class SPARRING_PROTOCOL_API USP_DeathScreen : public USP_MenuScreen
{
	GENERATED_BODY()

public:
	/** Call before AddToViewport (or after: the screen refreshes). */
	UFUNCTION(BlueprintCallable, Category = "Death Screen")
	void ShowResults(int32 InRoundsWon, int32 InRoundReached);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Screen")
	FName MainMenuLevelName = TEXT("L_MainMenu");

	/** Lines of the Hall of Fame panel. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Screen")
	int32 HallOfFameRows = 5;

	/** Position of the Hall of Fame panel from the top right corner. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Screen")
	FVector2D HallOfFameOffset = FVector2D(-160.f, 300.f);

	/** Random flicker of the "O" (Title_O). Turn off if the O is animated in Flicker. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Screen")
	bool bRandomFlickerOnO = true;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual UWidget* GetDefaultFocusWidget() const override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KnockoutText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> NewBestBadge;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> NameEntry;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Letter1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Letter2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Letter3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USP_MenuButton> RetryButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USP_MenuButton> MainMenuButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Title_O;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Flicker;

private:
	UFUNCTION()
	void HandleRetry();

	UFUNCTION()
	void HandleMainMenu();

	USP_LeaderboardSubsystem* GetLeaderboard() const;
	void RefreshAll();
	void RefreshLetters();
	void RefreshHallOfFame();
	void CycleLetter(int32 Direction);
	void MoveCursor(int32 Direction);
	void TypeLetter(TCHAR Letter);
	void ConfirmName();

	UPROPERTY(Transient)
	TObjectPtr<USP_ScoreList> HallOfFame;

	int32 RoundsWon = 0;
	int32 RoundReached = 1;
	bool bIsNewBest = false;
	bool bEnteringName = false;
	bool bConstructed = false;
	FString PlayerName = TEXT("AAA");
	int32 ActiveLetter = 0;
	int32 SavedIndex = INDEX_NONE;

	float FlickerTimer = 1.f;
	bool bOFlickerOff = false;
};
