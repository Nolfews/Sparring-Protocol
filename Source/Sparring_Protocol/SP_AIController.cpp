// Fill out your copyright notice in the Description page of Project Settings.


#include "SP_AIController.h"
#include "Kismet/GameplayStatics.h"


ASP_AIController::ASP_AIController()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASP_AIController::BeginPlay()
{
    Super::BeginPlay();
}

void ASP_AIController::UpdateTargetActor()
{
    UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
    if (!BlackboardComponent)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    UObject* CurrentTarget = BlackboardComponent->GetValueAsObject(FName("TargetActor"));
    if (CurrentTarget != PlayerPawn)
    {
        BlackboardComponent->SetValueAsObject(FName("TargetActor"), PlayerPawn);
    }
}

void ASP_AIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }

    UpdateTargetActor();
    GetWorldTimerManager().SetTimer(
        TargetUpdateTimerHandle,
        this,
        &ASP_AIController::UpdateTargetActor,
        0.1f,
        true);
}

void ASP_AIController::OnUnPossess()
{
    GetWorldTimerManager().ClearTimer(TargetUpdateTimerHandle);
    Super::OnUnPossess();
}