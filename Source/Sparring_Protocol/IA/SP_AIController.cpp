// Fill out your copyright notice in the Description page of Project Settings.


#include "SP_AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"


ASP_AIController::ASP_AIController()
{
    // Must tick: AAIController::Tick updates the control rotation from the focus (where the AI looks,
    // which also aims the sight cone and turns the body through bUseControllerDesiredRotation).
    PrimaryActorTick.bCanEverTick = true;

    UAIPerceptionComponent *Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    SetPerceptionComponent(*Perception);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.f;
	SightConfig->LoseSightRadius = 1800.f;
	SightConfig->PeripheralVisionAngleDegrees = 60.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	Perception->ConfigureSense(*SightConfig);
	Perception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ASP_AIController::BeginPlay()
{
    Super::BeginPlay();

    if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
    {
        Perception->OnTargetPerceptionUpdated.AddDynamic(this, &ASP_AIController::HandleTargetPerceptionUpdated);
    }
}

void ASP_AIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    const APawn* SensedPawn = Cast<APawn>(Actor);
    UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();

    if ((!SensedPawn || !BlackboardComponent) || !SensedPawn->IsPlayerControlled())
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        SetFocus(Actor);
        BlackboardComponent->SetValueAsObject(TargetActorKey, Actor);
        BlackboardComponent->ClearValue(LastKnownLocationKey);
    }
    else
    {
        BlackboardComponent->ClearValue(TargetActorKey);
        BlackboardComponent->SetValueAsVector(LastKnownLocationKey, Stimulus.StimulusLocation);
        ClearFocus(EAIFocusPriority::Gameplay);
    }
}

void ASP_AIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }
}
