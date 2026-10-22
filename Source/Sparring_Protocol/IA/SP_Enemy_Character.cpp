// Fill out your copyright notice in the Description page of Project Settings.


#include "SP_Enemy_Character.h"
#include "SP_AIController.h"
#include "IA/Components/SP_MovementPaceComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
ASP_Enemy_Character::ASP_Enemy_Character()
{
	AIControllerClass = ASP_AIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	MovementPace = CreateDefaultSubobject<USP_MovementPaceComponent>(TEXT("MovementPace"));

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);

	BaseEyeHeight = 70.f;
}

// Called when the game starts or when spawned
void ASP_Enemy_Character::BeginPlay()
{
	Super::BeginPlay();
}



// Called to bind functionality to input
void ASP_Enemy_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

