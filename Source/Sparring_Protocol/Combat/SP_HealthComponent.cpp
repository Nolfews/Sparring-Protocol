#include "Combat/SP_HealthComponent.h"

#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

USP_HealthComponent::USP_HealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USP_HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	bIsDead = false;

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &USP_HealthComponent::HandleTakeAnyDamage);
	}
}

void USP_HealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.f || bIsDead || bInvulnerable)
	{
		return;
	}

	const float OldHealth = Health;
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(this, Health, Health - OldHealth);

	if (bShowDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange,
			FString::Printf(TEXT("%s: -%.0f HP (%.0f / %.0f)"), *GetNameSafe(GetOwner()), Damage, Health, MaxHealth));
	}

	if (Health <= 0.f)
	{
		bIsDead = true;

		if (bShowDebugMessages && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("%s is dead"), *GetNameSafe(GetOwner())));
		}

		OnDeath.Broadcast(DamageCauser);
	}
}

void USP_HealthComponent::Heal(float Amount)
{
	if (Amount <= 0.f || bIsDead)
	{
		return;
	}

	const float OldHealth = Health;
	Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(this, Health, Health - OldHealth);
}

void USP_HealthComponent::ResetHealth()
{
	const float OldHealth = Health;
	Health = MaxHealth;
	bIsDead = false;
	OnHealthChanged.Broadcast(this, Health, Health - OldHealth);
}
