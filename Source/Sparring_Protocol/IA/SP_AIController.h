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
	virtual void OnUnPossess() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;   // keep the SAME name: your BP value is preserved

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	/**
	 * Half-angle of the sight cone: 60 = 120° in front, 90 = the whole front half, 180 = all around.
	 * The distance comes from the progression Data Asset (Base Sight Radius + unlocks).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "AI|Perception", meta = (ClampMin = "1", ClampMax = "180", Units = "Degrees"))
	float SightHalfAngle = 80.f;

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

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName CanLookAroundKey = TEXT("bCanLookAround");

	/** X-ray tier: how far the enemy feels the player through walls. */
	UPROPERTY(EditDefaultsOnly, Category = "AI|XRay", meta = (ClampMin = "0"))
	float XRayRange = 5000.f;

	/** Seconds between two updates of the felt position. */
	UPROPERTY(EditDefaultsOnly, Category = "AI|XRay", meta = (ClampMin = "0.05"))
	float XRayInterval = 0.25f;

private:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	void SetXRayEnabled(bool bEnabled);
	void UpdateXRay();

	FTimerHandle XRayTimer;
};