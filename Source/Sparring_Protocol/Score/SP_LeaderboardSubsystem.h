#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SP_LeaderboardSubsystem.generated.h"

/** One line of the local top 10. */
USTRUCT(BlueprintType)
struct SPARRING_PROTOCOL_API FSPLeaderboardEntry
{
	GENERATED_BODY()

	/** 3 letters, arcade style. */
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	FString Name;

	/** Rounds won in a row during the run. */
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	int32 Rounds = 0;
};

/** What is written on disk (Saved/SaveGames/Leaderboard.sav). */
UCLASS()
class SPARRING_PROTOCOL_API USP_LeaderboardSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FSPLeaderboardEntry> Entries;

	/** Name typed last time, proposed again on the next death screen. */
	UPROPERTY()
	FString LastName;
};

/**
 * Local leaderboard (top 10), loaded once when the game starts and saved after each new score.
 * Access: GetGameInstance()->GetSubsystem<USP_LeaderboardSubsystem>(), or "Get SP Leaderboard Subsystem" in Blueprint.
 */
UCLASS()
class SPARRING_PROTOCOL_API USP_LeaderboardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxEntries = 10;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Best first. */
	UFUNCTION(BlueprintPure, Category = "Leaderboard")
	TArray<FSPLeaderboardEntry> GetEntries() const;

	/** 0 if the board is empty. */
	UFUNCTION(BlueprintPure, Category = "Leaderboard")
	int32 GetBestRounds() const;

	/** True if this score enters the top 10. */
	UFUNCTION(BlueprintPure, Category = "Leaderboard")
	bool QualifiesForBoard(int32 Rounds) const;

	/** Where this score would be placed (after the equal scores already there). */
	UFUNCTION(BlueprintPure, Category = "Leaderboard")
	int32 GetInsertIndex(int32 Rounds) const;

	/** Adds the score, saves, and returns its position (or INDEX_NONE if it is not in the top 10). */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	int32 AddEntry(const FString& Name, int32 Rounds);

	UFUNCTION(BlueprintPure, Category = "Leaderboard")
	FString GetLastName() const;

	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void ClearLeaderboard();

	/** Uppercase letters only, exactly 3 characters (A-Z), "AAA" by default. */
	static FString SanitizeName(const FString& Name);

private:
	void Save();

	UPROPERTY(Transient)
	TObjectPtr<USP_LeaderboardSaveGame> SaveData;
};
