#include "IA/Progression/SP_EnemyProgressionComponent.h"
#include "IA/Progression/SP_EnemyProgressionConfig.h"
#include "Abilities/SP_ShootComponent.h"
#include "Combat/SP_HealthComponent.h"
#include "Game/GameFlowManager.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnemyProgression, Log, All);


USP_EnemyProgressionComponent::USP_EnemyProgressionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USP_EnemyProgressionComponent::BeginPlay()
{
    Super::BeginPlay();

    TActorIterator<AGameFlowManager> It(GetWorld());
    if (It)
    {
        GameFlow = *It;
        GameFlow->OnEnemySpawned.AddDynamic(this, &USP_EnemyProgressionComponent::HandleEnemySpawned);
    } else {
        ApplyLevel(ResolveLevel());
    }
}

void USP_EnemyProgressionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GameFlow)
    {
        GameFlow->OnEnemySpawned.RemoveDynamic(this, &USP_EnemyProgressionComponent::HandleEnemySpawned);
    }

    Super::EndPlay(EndPlayReason);
}

void USP_EnemyProgressionComponent::HandleEnemySpawned(APawn* EnemyPawn)
{
    if (EnemyPawn == GetOwner())
    {
        ApplyLevel(ResolveLevel());
    }
}

int32 USP_EnemyProgressionComponent::ResolveLevel() const
{
    if (DebugLevelOverride > 0)
    {
        return DebugLevelOverride;
    }
    return GameFlow ? FMath::Max(GameFlow->GetCurrentRound(), 1) : 1;
}

void USP_EnemyProgressionComponent::ApplyLevel(int32 NewLevel)
{
    if(!Config)
    {
        UE_LOG(LogEnemyProgression, Warning, TEXT("%s Has no progression config "), *GetNameSafe(GetOwner()));
        return;
    }

    CurrentLevel = FMath::Max(NewLevel, 1);

    const float MaxHealth = Config->MaxHealth.Evaluate(CurrentLevel);
    const float Damage = Config->Damage.Evaluate(CurrentLevel);

    if (USP_HealthComponent* HealthComp = GetOwner()->FindComponentByClass<USP_HealthComponent>())
    {
        HealthComp->MaxHealth = MaxHealth;
        HealthComp->ResetHealth();
    }

    if(USP_ShootComponent* ShootComp = GetOwner()->FindComponentByClass<USP_ShootComponent>())
    {
        ShootComp->Damage = Damage;
    }
    if (bShowDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan,
			FString::Printf(TEXT("ENEMY LEVEL %d | %.0f HP | %.1f damage"), CurrentLevel, MaxHealth, Damage));
	}
}