#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SP_AttackComponent.generated.h"

class UAnimMontage;

/** Broadcast on the frame the attack lands (set by the AnimNotify). The future weapon system listens here. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackFired);

/** Plays the attack animation of its owner. Shared by the player and the AI; knows nothing about weapons. */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API USP_AttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USP_AttackComponent();

	/** Plays the attack montage. False if already attacking or not set up. */
	UFUNCTION(BlueprintCallable, Category = "Attack")
	bool StartAttack();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StopAttack();

	UFUNCTION(BlueprintPure, Category = "Attack")
	bool IsAttacking() const { return bIsAttacking; }

	/** Called by the AttackFired AnimNotify. */
	void NotifyAttackFired();

	UPROPERTY(BlueprintAssignable, Category = "Attack|Events")
	FOnAttackFired OnAttackFired;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** Draws a line from the eyes when the attack lands, to check the notify timing. */
	UPROPERTY(EditAnywhere, Category = "Attack|Debug")
	bool bDrawDebugAttack = true;

private:
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	bool bIsAttacking = false;
};
