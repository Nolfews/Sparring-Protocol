// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SP_AIController.generated.h"

/**
 * 
 */
UCLASS()

class SPARRING_PROTOCOL_API ASP_AIController : public AAIController
{
	GENERATED_BODY()

public:
	ASP_AIController();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void BeginPlay() override;

private:
	void UpdateTargetActor();

	FTimerHandle TargetUpdateTimerHandle;

};