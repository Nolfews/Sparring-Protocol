#include "Abilities/SP_JumpComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void USP_JumpComponent::BeginPlay()
{
	Super::BeginPlay();
	ApplySettings();
}

void USP_JumpComponent::ApplySettings()
{
	ACharacter* Character = GetCharacter();
	UCharacterMovementComponent* Movement = GetMovement();
	if (!Character || !Movement)
	{
		return;
	}

	Character->JumpMaxCount = MaxJumps;
	Character->JumpMaxHoldTime = MaxHoldTime;
	Movement->JumpZVelocity = JumpVelocity;
	Movement->AirControl = AirControl;
}

void USP_JumpComponent::ActivateAbility()
{
	ACharacter* Character = GetCharacter();

	// A crouched character cannot use the normal Jump() (it is blocked by the engine).
	// This happens while sliding: stand up and launch upward, keeping the horizontal speed.
	if (Character->bIsCrouched)
	{
		Character->UnCrouch();
		Character->LaunchCharacter(FVector(0.f, 0.f, JumpVelocity), false, true);
		EndAbility();
		return;
	}

	// Normal jump. The engine handles the jump count (double jump) and the hold time.
	// The ability stays active until the button is released.
	Character->Jump();
}

void USP_JumpComponent::HandleRelease()
{
	EndAbility();
}

void USP_JumpComponent::OnAbilityEnd(bool bWasCancelled)
{
	if (ACharacter* Character = GetCharacter())
	{
		Character->StopJumping();
	}
}
