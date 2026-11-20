#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Score/SP_LeaderboardSubsystem.h"
#include "SP_ScoreList.generated.h"

class UTextBlock;
class UVerticalBox;

/**
 * "Hall of Fame" panel, built entirely in code (no Blueprint needed).
 * Shows the best scores, one line highlighted in green (the player's score).
 * Used by the death screen (top 5) and the leaderboard screen (top 10).
 */
UCLASS()
class SPARRING_PROTOCOL_API USP_ScoreList : public UUserWidget
{
	GENERATED_BODY()

public:
	/** HighlightIndex: line drawn in green (INDEX_NONE for none). */
	void SetEntries(const TArray<FSPLeaderboardEntry>& InEntries, int32 InHighlightIndex);

	/** Number of lines (empty lines are shown as "---"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score List")
	int32 MaxRows = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score List", meta = (ClampMin = "200"))
	float PanelWidth = 560.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score List")
	FText Title = NSLOCTEXT("SPMenu", "HallOfFame", "HALL OF FAME");

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildFrame();
	void RefreshRows();

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> Rows;

	TArray<FSPLeaderboardEntry> Entries;
	int32 HighlightIndex = INDEX_NONE;
};
