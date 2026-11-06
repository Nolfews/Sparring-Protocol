// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "SP_AIController.generated.h"


class UAISenseConfig_Sight;
struct FSPEnemyLevelData;
/**
 *
 */
UCLASS()

class SPARRING_PROTOCOL_API ASP_AIController : public AAIController
{
	GENERATED_BODY()

public:
	ASP_AIController();
	void ApplyLevelData(const FSPEnemyLevelData& LevelData);

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;   // keep the SAME name: your BP value is preserved

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	/** Blackboard key names, editable so a designer can rename keys without touching C++. */
	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName TargetActorKey = TEXT("TargetActor");

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName LastKnownLocationKey = TEXT("LastKnowLocation");

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName AttackCooldownKey = TEXT("AttackCooldown");

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName CanSearchKey = TEXT("bCanSearch");

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName CanRunandShootKey = TEXT("bCanRunandShoot");

private:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};