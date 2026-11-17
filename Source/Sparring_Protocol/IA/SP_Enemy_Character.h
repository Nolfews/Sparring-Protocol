// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/SP_CombatCharacter.h"
#include "SP_Enemy_Character.generated.h"

class USP_MovementPaceComponent;
class USP_AttackComponent;
class USP_EnemyProgressionComponent;
class USP_EnemyMobilityComponent;
class USkeletalMeshComponent;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USP_EnemyMobilityComponent> Mobility;

	/** Hidden until the visible rifle tier. The mesh is chosen in the Blueprint. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> Rifle;

	/**
	 * The visible body (the robot), attached to Mesh. Its Anim Blueprint copies the pose of Mesh
	 * every frame (Retarget Pose From Mesh). When it has a mesh, Mesh becomes an invisible driver
	 * that keeps playing ABP_Enemy, the montages, the notifies and the sockets (rifle, muzzle).
	 * Empty = Mesh stays visible, as before.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> VisualMesh;

	/**
	 * Socket of VisualMesh that holds the rifle (added on hand_r of the visual mesh skeleton).
	 * When it exists, the rifle follows the visible hand instead of the hidden driver hand.
	 * Missing = the rifle stays on HandGrip_R of Mesh.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName VisualRifleSocketName = TEXT("RifleGrip");

	/** The enemy spawns (and revives) with its outline. Dead = no outline. */
	UPROPERTY(EditDefaultsOnly, Category = "Outline")
	bool bOutlined = true;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName RifleMuzzleSocketName = TEXT("Muzzle");

	/** Used while the rifle is hidden. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName HandMuzzleSocketName = TEXT("weapon_r_muzzle");

	/** Where the projectiles leave. */
	FVector GetMuzzleLocation() const;

	/** The attack animation reached its AttackFired notify: the shoot ability (if any) fires a shot. */
	UFUNCTION()
	void HandleAttackFired();

public:
	ASP_Enemy_Character();

	virtual void Revive() override;

	/**
	 * Shows or hides the outline. It writes the visible body and the rifle into the custom depth,
	 * the outline post process of the player camera draws the edges of what it finds there.
	 */
	UFUNCTION(BlueprintCallable, Category = "Outline")
	void SetOutlineVisible(bool bVisible);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
