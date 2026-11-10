#include "IA/Progression/SP_EnemyProgressionConfig.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnemyBalance, Log, All);

FSPEnemyLevelData USP_EnemyProgressionConfig::GetLevelData(int32 Level) const
{
    FSPEnemyLevelData Data;
    Data.MaxHealth = MaxHealth.Evaluate(Level);
    Data.Damage = Damage.Evaluate(Level);
    Data.WalkSpeed = BaseWalkSpeed;
    Data.RunSpeed = BaseRunSpeed;
    Data.SightRadius = BaseSightRadius;
    Data.SpreadAngle = BaseSpreadAngle;
    Data.AttackCooldown = BaseAttackCooldown;
    Data.ProjectileSpeed = BaseProjectileSpeed;

    // The tiers stack: every tier reached adds its changes.
    for (const FSPEnemyUnlock& Unlock : Unlocks)
    {
        if (Level < Unlock.Level)
        {
            continue;
        }

        Data.WalkSpeed += Unlock.WalkSpeedBonus;
        Data.RunSpeed += Unlock.RunSpeedBonus;
        Data.SightRadius += Unlock.SightRadiusBonus;
        Data.SpreadAngle *= Unlock.SpreadMultiplier;
        Data.AttackCooldown *= Unlock.AttackCooldownMultiplier;
        Data.AttackPlayRate *= Unlock.AttackPlayRateMultiplier;
        Data.bCanSearch = Data.bCanSearch || Unlock.bUnlockSearch;
        Data.bCanRunandShoot = Data.bCanRunandShoot || Unlock.bUnlockRunandShoot;
        Data.MaxJumps = FMath::Max(Data.MaxJumps, Unlock.UnlockedJumps);
        Data.bCanDash = Data.bCanDash || Unlock.bUnlockDash;
        Data.ProjectileSpeed *= Unlock.ProjectileSpeedMultiplier;
        Data.bHasXRay = Data.bHasXRay || Unlock.bUnlockXRay;
        Data.bCanLookAround = Data.bCanLookAround || Unlock.bUnlockLookAround;
    }

    Data.LoseSightRadius = Data.SightRadius + LoseSightMargin;
    return Data;
}

void USP_EnemyProgressionConfig::PrintLevelTable() const
{
    UE_LOG(LogEnemyBalance, Display, TEXT("LVL |   HP |  DMG | WALK/RUN | SIGHT | SPREAD |   CD | BULLET | HITS TO KILL PLAYER | PLAYER SHOTS (TIME)"));

    for (int32 Level = 1; Level <= PreviewMaxLevel; ++Level)
    {
        const FSPEnemyLevelData Data = GetLevelData(Level);

        // The duel: how many enemy bullets the player survives, how many player bullets the enemy survives.
        const int32 HitsToKillPlayer = Data.Damage > 0.f ? FMath::CeilToInt(PreviewPlayerHealth / Data.Damage) : 0;
        const int32 ShotsToKillEnemy = FMath::CeilToInt(Data.MaxHealth / PreviewPlayerDamage);
        const float SecondsToKillEnemy = ShotsToKillEnemy / PreviewPlayerFireRate;

        const bool bIsTier = Unlocks.ContainsByPredicate([Level](const FSPEnemyUnlock& Unlock)
        {
            return Unlock.Level == Level;
        });

        UE_LOG(LogEnemyBalance, Display, TEXT("%3d | %4.0f | %4.1f |  %3.0f/%3.0f |  %4.0f |   %4.1f | %4.2f |   %4.0f | %19d | %3d (%.1f s)%s"),
            Level, Data.MaxHealth, Data.Damage, Data.WalkSpeed, Data.RunSpeed, Data.SightRadius,
            Data.SpreadAngle, Data.AttackCooldown, Data.ProjectileSpeed,
            HitsToKillPlayer, ShotsToKillEnemy, SecondsToKillEnemy,
            bIsTier ? TEXT("  <- tier") : TEXT(""));
    }
}