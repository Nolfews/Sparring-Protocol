#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SP_CombatCharacter.generated.h"

class USP_AbilityComponent;
class USP_HealthComponent;

/**
 * Shared base of the player and the AI (see README, "Shared Character Base Class").
 *
 *   ASP_CombatCharacter
 *    |- ASP_PlayerCharacter  (camera + Enhanced Input)
 *    |- enemy character      (Behavior Tree)  <- to reparent later
 *
 * It owns a health component. The abilities (USP_AbilityComponent subclasses: jump, slide,
 * dash, shoot...) are added freely in the Blueprint, with the "+ Add" button of the
 * Components panel. On death, every ability is disabled and the movement stops.
 */
UCLASS(Abstract)
class SPARRING_PROTOCOL_API ASP_CombatCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASP_CombatCharacter();

	UFUNCTION(BlueprintPure, Category = "Combat")
	USP_HealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsDead() const;

	/** Returns the first ability of this class (or null). Ex: FindAbility(SP_DashComponent). */
	UFUNCTION(BlueprintPure, Category = "Abilities", meta = (DeterminesOutputType = "AbilityClass"))
	USP_AbilityComponent* FindAbility(TSubclassOf<USP_AbilityComponent> AbilityClass) const;

	/** C++ version: FindAbilityByClass<USP_DashComponent>() */
	template <class T>
	T* FindAbilityByClass() const { return FindComponentByClass<T>(); }

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void SetAllAbilitiesEnabled(bool bEnabled);

protected:
	virtual void BeginPlay() override;

	/** Called when the health reaches 0. Override it in C++ (call Super). */
	UFUNCTION()
	virtual void HandleDeath(AActor* Killer);

	/** Blueprint hook for visuals only (death animation, sound, ragdoll). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat", meta = (DisplayName = "On Died"))
	void K2_OnDied(AActor* Killer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USP_HealthComponent> HealthComponent;
};
