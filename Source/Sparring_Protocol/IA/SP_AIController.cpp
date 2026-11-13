// Fill out your copyright notice in the Description page of Project Settings.


#include "SP_AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "IA/Progression/SP_EnemyProgressionConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"


ASP_AIController::ASP_AIController()
{
    // Must tick: AAIController::Tick updates the control rotation from the focus (where the AI looks,
    // which also aims the sight cone and turns the body through bUseControllerDesiredRotation).
    PrimaryActorTick.bCanEverTick = true;

    UAIPerceptionComponent *Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    SetPerceptionComponent(*Perception);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2000.f;
	SightConfig->LoseSightRadius = 2400.f;
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

void ASP_AIController::ApplyLevelData(const FSPEnemyLevelData& LevelData)
{
    if (SightConfig)
    {
        SightConfig->SightRadius = LevelData.SightRadius;
        SightConfig->LoseSightRadius = LevelData.LoseSightRadius;

        if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
        {
            Perception->ConfigureSense(*SightConfig);
        }
    }

    if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
    {
        BlackboardComponent->SetValueAsFloat(AttackCooldownKey, LevelData.AttackCooldown);
        BlackboardComponent->SetValueAsBool(CanSearchKey, LevelData.bCanSearch);
        BlackboardComponent->SetValueAsBool(CanRunandShootKey, LevelData.bCanRunandShoot);
        BlackboardComponent->SetValueAsBool(CanLookAroundKey, LevelData.bCanLookAround);
    }
    SetXRayEnabled(LevelData.bHasXRay);
}

void ASP_AIController::OnUnPossess()
{
    SetXRayEnabled(false);
    Super::OnUnPossess();
}

void ASP_AIController::SetXRayEnabled(bool bEnabled)
{
    FTimerManager& Timers = GetWorldTimerManager();

    if (!bEnabled)
    {
        Timers.ClearTimer(XRayTimer);
        return;
    }

    if (!Timers.IsTimerActive(XRayTimer))
    {
        Timers.SetTimer(XRayTimer, this, &ASP_AIController::UpdateXRay, XRayInterval, true);
    }
}

void ASP_AIController::UpdateXRay()
{
    UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
    const APawn* ControlledPawn = GetPawn();
    if (!BlackboardComponent || !ControlledPawn)
    {
        return;
    }

    // The eyes keep the priority: a seen player is chased, and only a seen player is shot.
    if (BlackboardComponent->GetValueAsObject(TargetActorKey))
    {
        return;
    }

    const ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player || Player->GetCharacterMovement()->IsFalling())
    {
        // In the air: keep the last position where the player stood.
        return;
    }

    if (FVector::Dist(Player->GetActorLocation(), ControlledPawn->GetActorLocation()) > XRayRange)
    {
        return;
    }

    BlackboardComponent->SetValueAsVector(LastKnownLocationKey, Player->GetActorLocation());
}