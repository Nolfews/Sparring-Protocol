#include "Game/GameFlowManager.h"
#include "Arena/ArenaManager.h"
#include "Placement/PlaceableDefinition.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogGameFlow, Log, All);

namespace
{
	constexpr uint64 HudKeyState = 8001;
	constexpr uint64 HudKeyInstruction = 8002;
	constexpr uint64 HudKeyDebugHelp = 8003;
	constexpr float HudDuration = 100000.f;
}

AGameFlowManager::AGameFlowManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGameFlowManager::BeginPlay()
{
	Super::BeginPlay();

	if (!ArenaManager)
	{
		TActorIterator<AArenaManager> It(GetWorld());
		if (It)
		{
			ArenaManager = *It;
		}
	}

	if (ArenaManager)
	{
		ArenaManager->OnBuildPhaseFinished.AddDynamic(this, &AGameFlowManager::HandleBuildPhaseFinished);
	}
	else
	{
		UE_LOG(LogGameFlow, Warning, TEXT("No ArenaManager found: the game flow cannot run."));
	}

	BindDebugInput();

	if (bAutoStartRun)
	{
		// Next tick, so every other BeginPlay (placement component, arena) is done and listening.
		GetWorldTimerManager().SetTimerForNextTick(this, &AGameFlowManager::StartRun);
	}
}

void AGameFlowManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ArenaManager)
	{
		ArenaManager->OnBuildPhaseFinished.RemoveDynamic(this, &AGameFlowManager::HandleBuildPhaseFinished);
	}

	if (DebugInput)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			PC->PopInputComponent(DebugInput);
		}
		DebugInput = nullptr;
	}

	if (GEngine)
	{
		GEngine->RemoveOnScreenDebugMessage(HudKeyState);
		GEngine->RemoveOnScreenDebugMessage(HudKeyInstruction);
		GEngine->RemoveOnScreenDebugMessage(HudKeyDebugHelp);
	}

	Super::EndPlay(EndPlayReason);
}

// ---- Flow ---------------------------------------------------------------------

void AGameFlowManager::StartRun()
{
	if (!ArenaManager)
	{
		UE_LOG(LogGameFlow, Warning, TEXT("StartRun: no ArenaManager."));
		return;
	}

	ArenaManager->ClearAllElements();
	RoundsWon = 0;
	CurrentRound = 0;

	if (!(bSkipTutorials || bCombatTutorialCompleted))
	{
		SetState(EGameFlowState::TutorialCombat);
		return;
	}

	BeginBuildStep();
}

void AGameFlowManager::CompleteCombatTutorial()
{
	if (State != EGameFlowState::TutorialCombat)
	{
		return;
	}
	bCombatTutorialCompleted = true;
	BeginBuildStep();
}

void AGameFlowManager::BeginBuildStep()
{
	if (!ArenaManager)
	{
		return;
	}

	// The guided build phase is only the first one of a run.
	const bool bTutorial = (RoundsWon == 0) && !(bSkipTutorials || bBuildTutorialCompleted);
	SetState(bTutorial ? EGameFlowState::TutorialBuild : EGameFlowState::Build);

	const int32 PlayerSlots = ArenaManager->BeginBuildPhase(RoundsWon);
	PlaceEnemyElements();

	if (PlayerSlots <= 0)
	{
		UE_LOG(LogGameFlow, Log, TEXT("No building slots for the player: skipping the build phase."));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow, TEXT("Arena is full: build phase skipped."));
		}
		ArenaManager->EndBuildPhase(); // Re-enters through HandleBuildPhaseFinished.
	}
}

void AGameFlowManager::HandleBuildPhaseFinished()
{
	if (State != EGameFlowState::TutorialBuild && State != EGameFlowState::Build)
	{
		return;
	}

	if (State == EGameFlowState::TutorialBuild)
	{
		bBuildTutorialCompleted = true;
	}
	StartCombatRound();
}

void AGameFlowManager::StartCombatRound()
{
	CurrentRound = RoundsWon + 1;
	SetState(EGameFlowState::RoundCombat);
	OnRoundStarted.Broadcast(CurrentRound);
}

void AGameFlowManager::NotifyRoundWon()
{
	if (State != EGameFlowState::RoundCombat)
	{
		return;
	}

	++RoundsWon;
	OnRoundEnded.Broadcast(CurrentRound, true);
	BeginBuildStep();
}

void AGameFlowManager::NotifyPlayerDefeated()
{
	if (State != EGameFlowState::RoundCombat)
	{
		return;
	}

	OnRoundEnded.Broadcast(CurrentRound, false);
	SetState(EGameFlowState::GameOver);
	OnRunEnded.Broadcast(RoundsWon);
	// TODO: save the score (rounds won in a row) to the local leaderboard.
}

void AGameFlowManager::PlaceEnemyElements()
{
	// No enemy construction during the tutorial, and none unless some elements are configured.
	if (State == EGameFlowState::TutorialBuild || EnemyDefinitions.IsEmpty() || !ArenaManager)
	{
		return;
	}

	int32 Safety = 32;
	while (ArenaManager->GetSlotsToPlace(EPlacementOwner::Enemy) > 0 && Safety-- > 0)
	{
		UPlaceableDefinition* Definition = EnemyDefinitions[FMath::RandRange(0, EnemyDefinitions.Num() - 1)];
		const FPlacementResult Result = ArenaManager->TryPlaceRandom(Definition, EPlacementOwner::Enemy);
		if (!Result.bSuccess)
		{
			break; // No valid place left: the enemy passes its turn.
		}
	}
}

void AGameFlowManager::SetState(EGameFlowState NewState)
{
	const EGameFlowState Previous = State;
	State = NewState;
	RefreshHud();
	OnStateChanged.Broadcast(NewState, Previous);
}

// ---- Debug --------------------------------------------------------------------

bool AGameFlowManager::CanRunDebugAction() const
{
	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, TEXT("Flow debug actions only work while playing (PIE)."));
		}
		return false;
	}
	return true;
}

void AGameFlowManager::Debug_NextStep()
{
	if (!CanRunDebugAction())
	{
		return;
	}

	switch (State)
	{
	case EGameFlowState::Idle:
	case EGameFlowState::GameOver:
		StartRun();
		break;
	case EGameFlowState::TutorialCombat:
		CompleteCombatTutorial();
		break;
	case EGameFlowState::TutorialBuild:
	case EGameFlowState::Build:
		if (ArenaManager)
		{
			ArenaManager->EndBuildPhase();
		}
		break;
	case EGameFlowState::RoundCombat:
		NotifyRoundWon();
		break;
	}
}

void AGameFlowManager::Debug_LoseRound()
{
	if (CanRunDebugAction())
	{
		NotifyPlayerDefeated();
	}
}

void AGameFlowManager::Debug_RestartRun()
{
	if (CanRunDebugAction())
	{
		StartRun();
	}
}

void AGameFlowManager::Debug_ResetTutorials()
{
	bCombatTutorialCompleted = false;
	bBuildTutorialCompleted = false;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("Tutorials will play again on the next run."));
	}
}

void AGameFlowManager::BindDebugInput()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	DebugInput = NewObject<UInputComponent>(PC, TEXT("GameFlowDebugInput"));
	DebugInput->Priority = 50;
	DebugInput->BindKey(NextStepKey, IE_Pressed, this, &AGameFlowManager::Debug_NextStep);
	DebugInput->BindKey(LoseRoundKey, IE_Pressed, this, &AGameFlowManager::Debug_LoseRound);
	DebugInput->BindKey(RestartKey, IE_Pressed, this, &AGameFlowManager::Debug_RestartRun);
	PC->PushInputComponent(DebugInput);
}

// ---- Debug HUD ----------------------------------------------------------------

void AGameFlowManager::RefreshHud() const
{
	if (!GEngine)
	{
		return;
	}

	const FString StateName = StaticEnum<EGameFlowState>()->GetNameStringByValue(static_cast<int64>(State));
	GEngine->AddOnScreenDebugMessage(HudKeyState, HudDuration, FColor::Cyan,
		FString::Printf(TEXT("FLOW: %s | Round: %d | Rounds won in a row: %d"), *StateName, CurrentRound, RoundsWon));

	FString Instruction;
	switch (State)
	{
	case EGameFlowState::TutorialCombat:
		Instruction = TEXT("TUTORIAL - COMBAT: learn to move, jump and shoot. The enemy does not fight back.");
		break;
	case EGameFlowState::TutorialBuild:
		Instruction = TEXT("TUTORIAL - BUILDING: shape the arena with your walls. You earn more slots after each round, but you can never block the only path between the spawns.");
		break;
	case EGameFlowState::Build:
		Instruction = TEXT("BUILD PHASE: place your elements, then finish building to start the next round.");
		break;
	case EGameFlowState::RoundCombat:
		Instruction = FString::Printf(TEXT("ROUND %d: defeat the enemy!"), CurrentRound);
		break;
	case EGameFlowState::GameOver:
		Instruction = FString::Printf(TEXT("GAME OVER - you won %d round(s) in a row."), RoundsWon);
		break;
	default:
		break;
	}
	GEngine->AddOnScreenDebugMessage(HudKeyInstruction, HudDuration, FColor::Yellow, Instruction);

	GEngine->AddOnScreenDebugMessage(HudKeyDebugHelp, HudDuration, FColor::Silver,
		FString::Printf(TEXT("DEBUG: [%s] next step / win round   [%s] lose round   [%s] restart run"),
			*NextStepKey.GetDisplayName().ToString(),
			*LoseRoundKey.GetDisplayName().ToString(),
			*RestartKey.GetDisplayName().ToString()));
}
