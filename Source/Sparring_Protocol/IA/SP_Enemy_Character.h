// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/SP_CombatCharacter.h"
#include "SP_Enemy_Character.generated.h"

class USP_MovementPaceComponent;
class USP_AttackComponent;
class USP_EnemyProgressionComponent;

/**
 * The enemy. Now built on ASP_CombatCharacter like the player: it gets a health component,
 * can carry ability components (the shoot ability) and handles its own death.
 */
UCLASS()
class SPARRING_PROTOCOL_API ASP_Enemy_Character : public ASP_CombatCharacter
{
	GENERATED_BODY()


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void HandleDeath(AActor* Killer) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USP_MovementPaceComponent> MovementPace;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USP_AttackComponent> Attack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USP_EnemyProgressionComponent> Progression;

	/** The attack animation reached its AttackFired notify: the shoot ability (if any) fires a shot. */
	UFUNCTION()
	void HandleAttackFired();

public:	
	ASP_Enemy_Character();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
