#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SP_EnemyProgressionComponent.generated.h"

class AGameFlowManager;
class APawn;
class USP_EnemyProgressionConfig;

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API USP_EnemyProgressionComponent : public UActorComponent
{
    GENERATED_BODY()

    public:

        USP_EnemyProgressionComponent();

        UFUNCTION(BlueprintCallable, Category = "Progression")
        void ApplyLevel(int32 NewLevel);

        UFUNCTION(BlueprintPure, Category = "Progression")
        int32 GetCurrentLevel() const {return CurrentLevel;}

    protected:
        virtual void BeginPlay() override;
        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
        TObjectPtr<USP_EnemyProgressionConfig> Config;

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression|Debug", meta = (ClampMin = "0"))
        int32 DebugLevelOverride = 0;

        UPROPERTY(EditAnywhere, Category = "Progression|Debug")
        bool bShowDebugMessages= true;

    private:

        UFUNCTION()
        void HandleEnemySpawned(APawn* EnemyPawn);

        int32 ResolveLevel() const;

        UPROPERTY(Transient)
        TObjectPtr<AGameFlowManager> GameFlow;
        int32 CurrentLevel = 0;


};