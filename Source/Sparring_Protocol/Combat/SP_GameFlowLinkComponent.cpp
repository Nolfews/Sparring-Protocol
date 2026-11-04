#include "Combat/SP_GameFlowLinkComponent.h"

#include "Abilities/SP_AbilityComponent.h"
#include "Abilities/SP_ShootComponent.h"
#include "Combat/SP_CombatCharacter.h"
#include "Combat/SP_HealthComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogSPGameFlowLink, Log, All);

namespace
{
	bool IsCombatState(EGameFlowState State)
	{
		return State == EGameFlowState::TutorialCombat || State == EGameFlowState::RoundCombat;
	}

	bool IsBuildState(EGameFlowState State)
	{
		return State == EGameFlowState::TutorialBuild || State == EGameFlowState::Build;
	}
}

USP_GameFlowLinkComponent::USP_GameFlowLinkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	AbilitiesDisabledWhileBuilding.Add(USP_ShootComponent::StaticClass());
}

void USP_GameFlowLinkComponent::BeginPlay()
{
	Super::BeginPlay();

	TActorIterator<AGameFlowManager> It(GetWorld());
	if (!It)
	{
		UE_LOG(LogSPGameFlowLink, Log, TEXT("%s: no GameFlowManager in the level, the link is inactive."), *GetNameSafe(GetOwner()));
		return;
	}
	GameFlow = *It;

	if (USP_HealthComponent* Health = GetOwner()->FindComponentByClass<USP_HealthComponent>())
	{
		Health->OnDeath.AddDynamic(this, &USP_GameFlowLinkComponent::HandleOwnerDeath);
	}
	else
	{
		UE_LOG(LogSPGameFlowLink, Warning, TEXT("%s has no SP_HealthComponent: its death cannot be reported to the game flow."), *GetNameSafe(GetOwner()));
	}

	if (Role == ESPCombatantRole::Player)
	{
		GameFlow->OnStateChanged.AddDynamic(this, &USP_GameFlowLinkComponent::HandleStateChanged);
		ApplyPlayerState(GameFlow->GetState());
	}
}

void USP_GameFlowLinkComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GameFlow)
	{
		GameFlow->OnStateChanged.RemoveDynamic(this, &USP_GameFlowLinkComponent::HandleStateChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void USP_GameFlowLinkComponent::HandleStateChanged(EGameFlowState NewState, EGameFlowState PreviousState)
{
	ApplyPlayerState(NewState);
}

void USP_GameFlowLinkComponent::ApplyPlayerState(EGameFlowState State)
{
	ASP_CombatCharacter* Character = Cast<ASP_CombatCharacter>(GetOwner());
	if (!Character || State == EGameFlowState::Idle)
	{
		return;
	}

	if (State == EGameFlowState::GameOver)
	{
		Character->SetAllAbilitiesEnabled(false);
		return;
	}

	// New combat (full health) or new run after a death: back on our feet.
	if (IsCombatState(State) || Character->IsDead())
	{
		Character->Revive();
	}
	else
	{
		Character->SetAllAbilitiesEnabled(true);
	}

	if (IsBuildState(State))
	{
		for (const TSubclassOf<USP_AbilityComponent>& AbilityClass : AbilitiesDisabledWhileBuilding)
		{
			if (USP_AbilityComponent* Ability = Character->FindAbility(AbilityClass))
			{
				Ability->SetAbilityEnabled(false);
			}
		}
	}
}

void USP_GameFlowLinkComponent::HandleOwnerDeath(AActor* Killer)
{
	if (!GameFlow)
	{
		return;
	}

	// Notified later, never during the death event itself: winning the round destroys the
	// enemy (and this component), which must not happen in the middle of its own broadcast.
	// The timer is tied to the GameFlowManager, so it survives the enemy being destroyed.
	const ESPCombatantRole DeadRole = Role;
	FTimerDelegate Notify = FTimerDelegate::CreateWeakLambda(GameFlow.Get(), [Flow = GameFlow.Get(), DeadRole]()
	{
		if (DeadRole == ESPCombatantRole::Player)
		{
			Flow->NotifyPlayerDefeated();
		}
		else if (Flow->GetState() == EGameFlowState::TutorialCombat)
		{
			Flow->CompleteCombatTutorial();
		}
		else
		{
			Flow->NotifyRoundWon();
		}
	});

	FTimerHandle Handle;
	if (DeathNotifyDelay > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(Handle, Notify, DeathNotifyDelay, false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(Notify);
	}
}
