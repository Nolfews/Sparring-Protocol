// Fill out your copyright notice in the Description page of Project Settings.


#include "SP_Enemy_Character.h"
#include "SP_AIController.h"


// Sets default values
ASP_Enemy_Character::ASP_Enemy_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = ASP_AIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

}

// Called when the game starts or when spawned
void ASP_Enemy_Character::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASP_Enemy_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASP_Enemy_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

