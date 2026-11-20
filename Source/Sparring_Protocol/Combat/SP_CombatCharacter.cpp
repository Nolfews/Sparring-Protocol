#include "Combat/SP_CombatCharacter.h"

#include "Abilities/SP_AbilityComponent.h"
#include "Combat/SP_HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ASP_CombatCharacter::ASP_CombatCharacter()
{
	HealthComponent = CreateDefaultSubobject<USP_HealthComponent>(TEXT("Health"));
}

void ASP_CombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnDeath.AddDynamic(this, &ASP_CombatCharacter::HandleDeath);
}

bool ASP_CombatCharacter::IsDead() const
{
	return HealthComponent && HealthComponent->IsDead();
}

USP_AbilityComponent* ASP_CombatCharacter::FindAbility(TSubclassOf<USP_AbilityComponent> AbilityClass) const
{
	if (!AbilityClass)
	{
		return nullptr;
	}
	return Cast<USP_AbilityComponent>(FindComponentByClass(AbilityClass));
}

void ASP_CombatCharacter::SetAllAbilitiesEnabled(bool bEnabled)
{
	TInlineComponentArray<USP_AbilityComponent*> Abilities(this);
	for (USP_AbilityComponent* Ability : Abilities)
	{
		Ability->SetAbilityEnabled(bEnabled);
	}
}

void ASP_CombatCharacter::Revive()
{
	HealthComponent->ResetHealth();
	SetAllAbilitiesEnabled(true);

	// DisableMovement() set the mode to None on death.
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (Movement && Movement->MovementMode == MOVE_None)
	{
		Movement->SetMovementMode(MOVE_Walking);
	}
}

void ASP_CombatCharacter::HandleDeath(AActor* Killer)
{
	SetAllAbilitiesEnabled(false);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	K2_OnDied(Killer);
}
