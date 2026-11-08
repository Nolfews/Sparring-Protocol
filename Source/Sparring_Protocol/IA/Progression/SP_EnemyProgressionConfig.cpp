#include "IA/Progression/SP_EnemyProgressionConfig.h"

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
    }

    Data.LoseSightRadius = Data.SightRadius + LoseSightMargin;
    return Data;
}