#include "Score/SP_LeaderboardSubsystem.h"

#include "Kismet/GameplayStatics.h"

namespace
{
	const TCHAR* LeaderboardSlot = TEXT("Leaderboard");
	constexpr int32 LeaderboardUserIndex = 0;
}

void USP_LeaderboardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameplayStatics::DoesSaveGameExist(LeaderboardSlot, LeaderboardUserIndex))
	{
		SaveData = Cast<USP_LeaderboardSaveGame>(UGameplayStatics::LoadGameFromSlot(LeaderboardSlot, LeaderboardUserIndex));
	}
	if (!SaveData)
	{
		SaveData = Cast<USP_LeaderboardSaveGame>(UGameplayStatics::CreateSaveGameObject(USP_LeaderboardSaveGame::StaticClass()));
	}
}

TArray<FSPLeaderboardEntry> USP_LeaderboardSubsystem::GetEntries() const
{
	return SaveData ? SaveData->Entries : TArray<FSPLeaderboardEntry>();
}

int32 USP_LeaderboardSubsystem::GetBestRounds() const
{
	return (SaveData && SaveData->Entries.Num() > 0) ? SaveData->Entries[0].Rounds : 0;
}

int32 USP_LeaderboardSubsystem::GetInsertIndex(int32 Rounds) const
{
	if (!SaveData)
	{
		return 0;
	}
	for (int32 Index = 0; Index < SaveData->Entries.Num(); ++Index)
	{
		if (SaveData->Entries[Index].Rounds < Rounds)
		{
			return Index;
		}
	}
	return SaveData->Entries.Num();
}

bool USP_LeaderboardSubsystem::QualifiesForBoard(int32 Rounds) const
{
	return Rounds > 0 && GetInsertIndex(Rounds) < MaxEntries;
}

int32 USP_LeaderboardSubsystem::AddEntry(const FString& Name, int32 Rounds)
{
	if (!SaveData)
	{
		return INDEX_NONE;
	}

	const FString CleanName = SanitizeName(Name);
	SaveData->LastName = CleanName;

	int32 Index = INDEX_NONE;
	if (QualifiesForBoard(Rounds))
	{
		Index = GetInsertIndex(Rounds);

		FSPLeaderboardEntry Entry;
		Entry.Name = CleanName;
		Entry.Rounds = Rounds;
		SaveData->Entries.Insert(Entry, Index);

		if (SaveData->Entries.Num() > MaxEntries)
		{
			SaveData->Entries.SetNum(MaxEntries);
		}
	}

	Save();
	return Index;
}

FString USP_LeaderboardSubsystem::GetLastName() const
{
	return (SaveData && !SaveData->LastName.IsEmpty()) ? SaveData->LastName : FString(TEXT("AAA"));
}

void USP_LeaderboardSubsystem::ClearLeaderboard()
{
	if (SaveData)
	{
		SaveData->Entries.Reset();
		Save();
	}
}

FString USP_LeaderboardSubsystem::SanitizeName(const FString& Name)
{
	FString Result;
	for (const TCHAR Character : Name.ToUpper())
	{
		if (Character >= TEXT('A') && Character <= TEXT('Z'))
		{
			Result.AppendChar(Character);
		}
		if (Result.Len() == 3)
		{
			break;
		}
	}
	while (Result.Len() < 3)
	{
		Result.AppendChar(TEXT('A'));
	}
	return Result;
}

void USP_LeaderboardSubsystem::Save()
{
	if (SaveData)
	{
		UGameplayStatics::SaveGameToSlot(SaveData, LeaderboardSlot, LeaderboardUserIndex);
	}
}
