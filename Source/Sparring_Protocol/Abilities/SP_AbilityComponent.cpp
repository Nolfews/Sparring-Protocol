#include "Abilities/SP_AbilityComponent.h"

#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogSPAbility, Log, All);

USP_AbilityComponent::USP_AbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USP_AbilityComponent::OnRegister()
{
	Super::OnRegister();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter && GetOwner())
	{
		UE_LOG(LogSPAbility, Warning, TEXT("%s must be added to a Character (owner: %s)."), *GetName(), *GetNameSafe(GetOwner()));
	}
}

void USP_AbilityComponent::BindInput(UEnhancedInputComponent* EnhancedInput)
{
	if (!EnhancedInput)
	{
		return;
	}

	if (!InputAction)
	{
		UE_LOG(LogSPAbility, Log, TEXT("%s has no InputAction: it can only be triggered from code."), *GetName());
		return;
	}

	EnhancedInput->BindAction(InputAction, ETriggerEvent::Started, this, &USP_AbilityComponent::OnInputStarted);
	EnhancedInput->BindAction(InputAction, ETriggerEvent::Completed, this, &USP_AbilityComponent::OnInputReleased);
	EnhancedInput->BindAction(InputAction, ETriggerEvent::Canceled, this, &USP_AbilityComponent::OnInputReleased);
}

void USP_AbilityComponent::OnInputStarted()
{
	TryActivate();
}

void USP_AbilityComponent::OnInputReleased()
{
	Release();
}

bool USP_AbilityComponent::CanActivate() const
{
	return bAbilityEnabled
		&& OwnerCharacter != nullptr
		&& !bIsAbilityActive
		&& GetCooldownRemaining() <= 0.f
		&& CanActivateAbility();
}

bool USP_AbilityComponent::TryActivate()
{
	if (!CanActivate())
	{
		return false;
	}

	bIsAbilityActive = true;
	OnAbilityActivated.Broadcast(this);
	ActivateAbility();
	return true;
}

void USP_AbilityComponent::Release()
{
	if (bIsAbilityActive)
	{
		HandleRelease();
	}
}

void USP_AbilityComponent::CancelAbility()
{
	if (bIsAbilityActive)
	{
		EndAbility(true);
	}
}

void USP_AbilityComponent::EndAbility(bool bWasCancelled)
{
	if (!bIsAbilityActive)
	{
		return;
	}

	bIsAbilityActive = false;
	OnAbilityEnd(bWasCancelled);

	if (Cooldown > 0.f)
	{
		CooldownEndTime = GetWorldTime() + Cooldown;
	}

	OnAbilityEnded.Broadcast(this);
}

float USP_AbilityComponent::GetCooldownRemaining() const
{
	return FMath::Max(0.f, static_cast<float>(CooldownEndTime - GetWorldTime()));
}

void USP_AbilityComponent::SetAbilityEnabled(bool bEnabled)
{
	bAbilityEnabled = bEnabled;
	if (!bEnabled)
	{
		CancelAbility();
	}
}

UCharacterMovementComponent* USP_AbilityComponent::GetMovement() const
{
	return OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
}

double USP_AbilityComponent::GetWorldTime() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.0;
}
