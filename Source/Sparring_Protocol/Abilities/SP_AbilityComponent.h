#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SP_AbilityComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UEnhancedInputComponent;
class UInputAction;
class USP_AbilityComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSPAbilityEvent, USP_AbilityComponent*, Ability);

/**
 * Base class of every ability (jump, slide, dash, shoot...).
 *
 * An ability is a component added to a Character, player or AI:
 *  - player: pressing InputAction calls TryActivate(), releasing it calls Release()
 *    (the binding is done automatically by ASP_PlayerCharacter)
 *  - AI: the Behavior Tree calls TryActivate() / Release() directly
 * so each ability is written once and works for both.
 *
 * To make a new ability, inherit from this class and override:
 *  - CanActivateAbility() : extra conditions (on ground, has charges...)
 *  - ActivateAbility()    : what happens on press
 *  - HandleRelease()      : what happens on release (optional)
 *  - OnAbilityEnd()       : clean up (restore movement values, timers...)
 * and call EndAbility() when the ability is done.
 */
UCLASS(Abstract, ClassGroup = (Abilities), Blueprintable)
class SPARRING_PROTOCOL_API USP_AbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USP_AbilityComponent();

	/** Called by the player character: binds InputAction to TryActivate / Release. */
	virtual void BindInput(UEnhancedInputComponent* EnhancedInput);

	/** Starts the ability if possible. Returns true if it started. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	bool TryActivate();

	/** The button was released (stop firing, stop sliding, variable jump height...). */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void Release();

	/** Stops the ability right now (death, end of round...). */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void CancelAbility();

	UFUNCTION(BlueprintPure, Category = "Ability")
	bool CanActivate() const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	bool IsAbilityActive() const { return bIsAbilityActive; }

	UFUNCTION(BlueprintPure, Category = "Ability")
	float GetCooldownRemaining() const;

	/** Disabling an ability also cancels it if it is running. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void SetAbilityEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Ability")
	bool IsAbilityEnabled() const { return bAbilityEnabled; }

	/** Input Action that triggers this ability for the player. Leave empty for AI-only use. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Input")
	TObjectPtr<UInputAction> InputAction;

	/** Time after the end of the ability before it can be used again. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (ClampMin = "0", Units = "s"))
	float Cooldown = 0.f;

	UPROPERTY(BlueprintAssignable, Category = "Ability|Events")
	FSPAbilityEvent OnAbilityActivated;

	UPROPERTY(BlueprintAssignable, Category = "Ability|Events")
	FSPAbilityEvent OnAbilityEnded;

protected:
	virtual void OnRegister() override;

	virtual bool CanActivateAbility() const { return true; }
	virtual void ActivateAbility() {}
	virtual void HandleRelease() {}
	virtual void OnAbilityEnd(bool bWasCancelled) {}

	/** Marks the ability as finished, starts the cooldown and fires OnAbilityEnded. */
	void EndAbility(bool bWasCancelled = false);

	ACharacter* GetCharacter() const { return OwnerCharacter; }
	UCharacterMovementComponent* GetMovement() const;
	double GetWorldTime() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	bool bAbilityEnabled = true;

private:
	void OnInputStarted();
	void OnInputReleased();

	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter;

	bool bIsAbilityActive = false;
	double CooldownEndTime = 0.0;
};
