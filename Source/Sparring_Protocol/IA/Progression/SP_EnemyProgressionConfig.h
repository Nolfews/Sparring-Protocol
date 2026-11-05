#pragma once


#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SP_EnemyProgressionConfig.generated.h"

USTRUCT(BlueprintType)

struct FSPScalingStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Base = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float PerLevel = 0.f;

    float Evaluate(int32 Level) const
    {
        return Base + PerLevel * FMath::Max(Level - 1, 0);
    }
};

UCLASS(BlueprintType)
class SPARRING_PROTOCOL_API USP_EnemyProgressionConfig : public UDataAsset
{
    GENERATED_BODY()

    public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    FSPScalingStats MaxHealth = {100.f, 20.f};

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    FSPScalingStats Damage = {5.f, 0.5f};
};