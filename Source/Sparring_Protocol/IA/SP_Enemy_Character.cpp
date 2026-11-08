// Fill out your copyright notice in the Description page of Project Settings.


#include "SP_Enemy_Character.h"
#include "SP_AIController.h"
#include "IA/Components/SP_MovementPaceComponent.h"
#include "IA/Components/SP_AttackComponent.h"
#include "Abilities/SP_ShootComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "IA/Progression/SP_EnemyProgressionComponent.h"
#include "IA/Components/SP_EnemyMobilityComponent.h"

// Sets default values
ASP_Enemy_Character::ASP_Enemy_Character()
{
	AIControllerClass = ASP_AIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	MovementPace = CreateDefaultSubobject<USP_MovementPaceComponent>(TEXT("MovementPace"));
	Attack = CreateDefaultSubobject<USP_AttackComponent>(TEXT("Attack"));
	Progression = CreateDefaultSubobject<USP_EnemyProgressionComponent>(TEXT("Progression"));
	Mobility = CreateDefaultSubobject<USP_EnemyMobilityComponent>(TEXT("Mobility"));

	// The body follows the controller rotation. The controller looks where the AI walks (move focus)
	// or at its target (gameplay focus): this lets the enemy face the player while strafing.
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);

	BaseEyeHeight = 70.f;
}

// Called when the game starts or when spawned
void ASP_Enemy_Character::BeginPlay()
{
	Super::BeginPlay();

	// The animation decides WHEN a shot leaves (AnimNotify), the shoot ability decides WHAT it does.
	if (Attack)
	{
		Attack->OnAttackFired.AddDynamic(this, &ASP_Enemy_Character::HandleAttackFired);
	}
}

void ASP_Enemy_Character::HandleAttackFired()
{
	if (USP_ShootComponent* Shoot = FindComponentByClass<USP_ShootComponent>())
	{
		Shoot->FireOnce();
	}
}

void ASP_Enemy_Character::HandleDeath(AActor* Killer)
{
	Super::HandleDeath(Killer);

	if (Attack)
	{
		Attack->StopAttack();
	}

	if (Mobility)
	{
		Mobility->StopClimbing();
	}


	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
		AIController->ClearFocus(EAIFocusPriority::Gameplay);

		if (UBrainComponent* Brain = AIController->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Enemy dead"));
		}
	}
}

// Called to bind functionality to input
void ASP_Enemy_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
