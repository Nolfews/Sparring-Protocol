// Fill out your copyright notice in the Description page of Project Settings.


#include "SP_AIController.h"
#include "Kismet/GameplayStatics.h"


ASP_AIController::ASP_AIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void  ASP_AIController::BeginPlay()
{
    Super::BeginPlay();

    /* APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
     if (PlayerPawn)
     {
         MoveToActor(PlayerPawn, 100.0f);

         if (GEngine)
         {
             GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("IA : Joueur trouve, deplacement en cours !"));
         }
     }
     else if (GEngine)
     {
         GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("IA : Joueur introuvable !"));
     }*/
}

void  ASP_AIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    if (PlayerPawn && GetBlackboardComponent())
    {

        GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), PlayerPawn);

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