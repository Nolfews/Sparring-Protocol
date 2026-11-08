#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SP_EnemyProgressionConfig.generated.h"

/** A stat that grows forever: Base at level 1, then PerLevel more at each level. */
USTRUCT(BlueprintType)
struct FSPScalingStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Base = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float PerLevel = 0.f;

    float Evaluate(int32 Level) const
    {
        return Base + PerLevel * FMath::Max(Level - 1, 0);
    }
};

/** What changes from a given level. Leave a field at its default when this tier does not touch it. */
USTRUCT(BlueprintType)
struct FSPEnemyUnlock
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock", meta = (ClampMin = "1"))
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock|Movement")
    float WalkSpeedBonus = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock|Movement")
    float RunSpeedBonus = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock|Sight")
    float SightRadiusBonus = 0.f;

    /** 0.5 = twice as accurate. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock|Attack", meta = (ClampMin = "0"))
    float SpreadMultiplier = 1.f;

    /** 0.5 = half the wait between two attacks. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock|Attack", meta = (ClampMin = "0"))
    float AttackCooldownMultiplier = 1.f;

    /** 1.5 = the attack animation plays 50% faster. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock|Attack", meta = (ClampMin = "0.1"))
    float AttackPlayRateMultiplier = 1.f;

    /** The enemy goes to the last known location of the player. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock|Behaviour")
    bool bUnlockSearch = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock|Behaviour")
    bool bUnlockRunandShoot = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock|Behaviour", meta = (ClampMin = "0"))
    int32 UnlockedJumps = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock|Behaviour")
    bool bUnlockDash = false;

    /** 1.5 = projectiles 50% faster. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock|Weapon", meta = (ClampMin = "0.1"))
    float ProjectileSpeedMultiplier = 1.f;

};

/** Every final value of one level, once the stats and the reached tiers are combined. */
USTRUCT(BlueprintType)
struct FSPEnemyLevelData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    float MaxHealth = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    float Damage = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    float WalkSpeed = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    float RunSpeed = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    float SightRadius = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    float LoseSightRadius = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    float SpreadAngle = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    float AttackCooldown = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    float AttackPlayRate = 1.f;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    bool bCanSearch = false;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    bool bCanRunandShoot = false;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    int32 MaxJumps = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    bool bCanDash = false;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    float ProjectileSpeed = 0.f;

};

/** Data Asset holding how the enemy grows each round. */
UCLASS(BlueprintType)
class SPARRING_PROTOCOL_API USP_EnemyProgressionConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Combines the stats and every tier reached at this level. */
    FSPEnemyLevelData GetLevelData(int32 Level) const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    FSPScalingStats MaxHealth = {100.f, 20.f};

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    FSPScalingStats Damage = {5.f, 0.5f};

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base", meta = (ClampMin = "0"))
    float BaseWalkSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base", meta = (ClampMin = "0"))
    float BaseRunSpeed = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base", meta = (ClampMin = "0"))
    float BaseSightRadius = 2000.f;

    /** The player is lost this far beyond the sight radius. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base", meta = (ClampMin = "0"))
    float LoseSightMargin = 400.f;

    /** Full cone of the shots, in degrees. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base", meta = (ClampMin = "0", ClampMax = "45"))
    float BaseSpreadAngle = 8.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base", meta = (ClampMin = "0"))
    float BaseAttackCooldown = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base", meta = (ClampMin = "100"))
    float BaseProjectileSpeed = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlocks")
    TArray<FSPEnemyUnlock> Unlocks;

};