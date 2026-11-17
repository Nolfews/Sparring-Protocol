#pragma once

#include "CoreMinimal.h"
#include "UI/SP_MenuScreen.h"
#include "SP_LeaderboardScreen.generated.h"

class USP_MenuButton;
class USP_ScoreList;

/**
 * Local top 10, opened from the main menu. Built entirely in code:
 * no Blueprint to make. BACK (or Escape / gamepad B) returns to the menu.
 */
UCLASS()
class SPARRING_PROTOCOL_API USP_LeaderboardScreen : public USP_MenuScreen
{
	GENERATED_BODY()

public:
	USP_LeaderboardScreen(const FObjectInitializer& ObjectInitializer);

	/** Look of the BACK button (WBP_MenuButton by default). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leaderboard")
	TSoftClassPtr<USP_MenuButton> MenuButtonClass;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual UWidget* GetDefaultFocusWidget() const override;
	virtual bool HandleBack() override;

private:
	void BuildTree();

	UFUNCTION()
	void HandleBackPressed();

	UPROPERTY(Transient)
	TObjectPtr<USP_ScoreList> ScoreList;

	UPROPERTY(Transient)
	TObjectPtr<USP_MenuButton> BackButton;
};
