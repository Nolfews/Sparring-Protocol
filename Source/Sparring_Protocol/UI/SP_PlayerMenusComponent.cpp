#include "UI/SP_PlayerMenusComponent.h"

#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "Game/GameFlowManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "UI/SP_DeathScreen.h"
#include "UI/SP_PauseScreen.h"

DEFINE_LOG_CATEGORY_STATIC(LogSPMenus, Log, All);

USP_PlayerMenusComponent::USP_PlayerMenusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	PauseAction = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Player/Input/IA_Pause.IA_Pause")));
	PauseScreenClass = TSoftClassPtr<USP_PauseScreen>(FSoftObjectPath(TEXT("/Game/UI/Widgets/WBP_PauseMenu.WBP_PauseMenu_C")));
	DeathScreenClass = TSoftClassPtr<USP_DeathScreen>(FSoftObjectPath(TEXT("/Game/UI/Widgets/WBP_DeathScreen.WBP_DeathScreen_C")));
}

void USP_PlayerMenusComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only the GameFlowManager of the arena ends runs; test maps have none, nothing to do there.
	TActorIterator<AGameFlowManager> It(GetWorld());
	if (It)
	{
		GameFlow = *It;
		GameFlow->OnRunEnded.AddDynamic(this, &USP_PlayerMenusComponent::HandleRunEnded);
	}
}

void USP_PlayerMenusComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GameFlow)
	{
		GameFlow->OnRunEnded.RemoveDynamic(this, &USP_PlayerMenusComponent::HandleRunEnded);
	}
	Super::EndPlay(EndPlayReason);
}

void USP_PlayerMenusComponent::BindInput(UEnhancedInputComponent* EnhancedInput)
{
	if (!EnhancedInput)
	{
		return;
	}

	if (UInputAction* Action = PauseAction.LoadSynchronous())
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &USP_PlayerMenusComponent::TogglePause);
	}
	else
	{
		UE_LOG(LogSPMenus, Warning, TEXT("Pause action not found (%s): the pause menu cannot be opened."), *PauseAction.ToString());
	}
}

APlayerController* USP_PlayerMenusComponent::GetPlayerController() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	return Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
}

void USP_PlayerMenusComponent::TogglePause()
{
	// No pause on top of the death screen.
	if (DeathScreen && DeathScreen->IsInViewport())
	{
		return;
	}

	APlayerController* PlayerController = GetPlayerController();
	if (!PlayerController)
	{
		return;
	}

	if (!PauseScreen)
	{
		UClass* ScreenClass = PauseScreenClass.LoadSynchronous();
		if (!ScreenClass)
		{
			UE_LOG(LogSPMenus, Warning, TEXT("Pause screen class not found (%s)."), *PauseScreenClass.ToString());
			return;
		}
		PauseScreen = CreateWidget<USP_PauseScreen>(PlayerController, ScreenClass);
	}

	if (PauseScreen->IsPauseShown())
	{
		PauseScreen->HidePause();
	}
	else
	{
		PauseScreen->ShowPause();
	}
}

void USP_PlayerMenusComponent::HandleRunEnded(int32 RoundsWon)
{
	ShowDeathScreen(RoundsWon);
}

void USP_PlayerMenusComponent::ShowDeathScreen(int32 RoundsWon)
{
	APlayerController* PlayerController = GetPlayerController();
	if (!PlayerController)
	{
		return;
	}

	if (PauseScreen && PauseScreen->IsPauseShown())
	{
		PauseScreen->HidePause();
	}

	if (!DeathScreen)
	{
		UClass* ScreenClass = DeathScreenClass.LoadSynchronous();
		if (!ScreenClass)
		{
			UE_LOG(LogSPMenus, Warning, TEXT("Death screen class not found (%s)."), *DeathScreenClass.ToString());
			return;
		}
		DeathScreen = CreateWidget<USP_DeathScreen>(PlayerController, ScreenClass);
	}

	const int32 RoundReached = GameFlow ? GameFlow->GetCurrentRound() : RoundsWon + 1;
	DeathScreen->ShowResults(RoundsWon, RoundReached);

	if (!DeathScreen->IsInViewport())
	{
		DeathScreen->AddToViewport(60);
	}
	DeathScreen->ActivateMenuInput(false);
	DeathScreen->FocusDefault();
}
