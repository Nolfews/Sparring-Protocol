#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SP_HealthComponent.generated.h"

class UDamageType;
class AController;
class USP_HealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSPOnHealthChanged, USP_HealthComponent*, HealthComponent, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSPOnDeath, AActor*, Killer);

/**
 * Health of any actor (player, AI, target dummy, destructible wall...).
 * Listens to the engine damage events (ApplyDamage / ApplyPointDamage), so the damage
 * dealer does not need to know this component exists.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API USP_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USP_HealthComponent();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	/** Back to MaxHealth (new round). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "1"))
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	bool bInvulnerable = false;

	/** Prints every hit on screen (for tests). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Debug")
	bool bShowDebugMessages = true;

	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FSPOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FSPOnDeath OnDeath;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	float Health = 0.f;
	bool bIsDead = false;
};
