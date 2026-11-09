#include "IA/Progression/SP_EnemyProgressionComponent.h"
#include "IA/Progression/SP_EnemyProgressionConfig.h"
#include "Abilities/SP_ShootComponent.h"
#include "Combat/SP_HealthComponent.h"
#include "Game/GameFlowManager.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "IA/Components/SP_AttackComponent.h"
#include "IA/Components/SP_MovementPaceComponent.h"
#include "IA/SP_AIController.h"
#include "IA/Components/SP_EnemyMobilityComponent.h"

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
    if (!Config)
    {
        UE_LOG(LogEnemyProgression, Warning, TEXT("%s Has no progression config "), *GetNameSafe(GetOwner()));
        return;
    }

    CurrentLevel = FMath::Max(NewLevel, 1);
    const FSPEnemyLevelData Data = Config->GetLevelData(CurrentLevel);

    // The body.
    if (USP_HealthComponent* HealthComp = GetOwner()->FindComponentByClass<USP_HealthComponent>())
    {
        HealthComp->MaxHealth = Data.MaxHealth;
        HealthComp->ResetHealth();
    }

    if (USP_ShootComponent* ShootComp = GetOwner()->FindComponentByClass<USP_ShootComponent>())
    {
        ShootComp->Damage = Data.Damage;
        ShootComp->SpreadAngle = Data.SpreadAngle;
        ShootComp->ProjectileSpeed = Data.ProjectileSpeed;
    }

    if (USP_MovementPaceComponent* PaceComp = GetOwner()->FindComponentByClass<USP_MovementPaceComponent>())
    {
        PaceComp->SetSpeeds(Data.WalkSpeed, Data.RunSpeed);
    }

    if (USP_AttackComponent* AttackComp = GetOwner()->FindComponentByClass<USP_AttackComponent>())
    {
        AttackComp->SetAttackPlayRate(Data.AttackPlayRate);
        AttackComp->SetAttackCooldown(Data.AttackCooldown);
    }

     if (USP_EnemyMobilityComponent* MobilityComp = GetOwner()->FindComponentByClass<USP_EnemyMobilityComponent>())
    {
        MobilityComp->SetCapabilities(Data.MaxJumps, Data.bCanDash, Data.bHasXRay);
    }

    // The brain.
    if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (ASP_AIController* AIController = Cast<ASP_AIController>(OwnerPawn->GetController()))
        {
            AIController->ApplyLevelData(Data);
        }
    }

    if (bShowDebugMessages && GEngine)
    {
         GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan,
            FString::Printf(TEXT("ENEMY LEVEL %d | %.0f HP | %.1f damage | %.0f/%.0f speed | %.0f sight | %.1f spread | %.2f s | search %s | run&shoot %s | jumps %d | dash %s | bullet %.0f | xray %s"),
                CurrentLevel, Data.MaxHealth, Data.Damage, Data.WalkSpeed, Data.RunSpeed,
                Data.SightRadius, Data.SpreadAngle, Data.AttackCooldown,
                Data.bCanSearch ? TEXT("on") : TEXT("off"),
                Data.bCanRunandShoot ? TEXT("on") : TEXT("off"),
                Data.MaxJumps,
                Data.bCanDash ? TEXT("on") : TEXT("off"),
                Data.ProjectileSpeed,
                Data.bHasXRay ? TEXT("on") : TEXT("off")));
    }
}