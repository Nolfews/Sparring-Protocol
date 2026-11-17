#pragma once

#include "CoreMinimal.h"
#include "Abilities/SP_AbilityComponent.h"
#include "Engine/EngineTypes.h"
#include "SP_ShootComponent.generated.h"

class UDamageType;
class USoundBase;
class ASP_CombatProjectile;

/** Fired once per shot (sound, muzzle flash, recoil, camera shake). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSPOnShotFired);

/** Fired for each trace of a shot (one per pellet): bullet trail, impact effect. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSPOnShotTrace, FVector, TraceStart, FVector, TraceEnd, const FHitResult&, Hit);

/**
 * Hitscan weapon: a line trace from the eyes (the camera for the player, the eyes and
 * aim rotation for the AI). Damage goes through the engine damage system, so any actor
 * with a USP_HealthComponent can be hurt.
 * Automatic (hold) or semi-automatic (one shot per click), spread for the AI accuracy,
 * several pellets for a shotgun.
 */
UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API USP_ShootComponent : public USP_AbilityComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot", meta = (ClampMin = "0"))
	float Damage = 20.f;

	/** Shots per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot", meta = (ClampMin = "0.1"))
	float FireRate = 8.f;

	/** True: keeps firing while the button is held. False: one shot per click. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot")
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot", meta = (ClampMin = "100", Units = "cm"))
	float Range = 10000.f;

	/** Cone angle of the spread. 0 = perfect accuracy (lower the accuracy of the AI with this). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot", meta = (ClampMin = "0", ClampMax = "45", Units = "Degrees"))
	float SpreadAngle = 0.f;

	/** More than 1 for a shotgun. Each pellet deals Damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot", meta = (ClampMin = "1"))
	int32 PelletsPerShot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Projectile")
	TSubclassOf<ASP_CombatProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Projectile", meta = (ClampMin = "100", Units = "CentimetersPerSecond"))
	float ProjectileSpeed = 3000.f;

	/**
	 * Played once per shot (not per pellet). The local player hears it in 2D (no panning on his own gun),
	 * everybody else hears it at the muzzle: set an attenuation in the Sound Cue so the distance counts.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Sound")
	TObjectPtr<USoundBase> FireSound;

	/** Small random pitch change on each shot so automatic fire does not sound like a loop. 0 = off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Sound", meta = (ClampMin = "0", ClampMax = "0.5"))
	float FirePitchVariation = 0.05f;

	/** Draws the impacts (red = hit something, green = nothing). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Debug")
	bool bDrawDebug = false;

	UPROPERTY(BlueprintAssignable, Category = "Shoot|Events")
	FSPOnShotFired OnShotFired;

	UPROPERTY(BlueprintAssignable, Category = "Shoot|Events")
	FSPOnShotTrace OnShotTrace;

	/**
	 * Fires one shot right now, with no fire-rate timer and no input. For the AI: its attack animation
	 * calls this on the frame the shot should leave. Does nothing while the ability is disabled (dead...).
	 */
	UFUNCTION(BlueprintCallable, Category = "Shoot")
	void FireOnce();

	UFUNCTION(BlueprintCallable, Category = "Shoot")
	void FireOnceFrom(const FVector& MuzzleLocation);

	/**
	 * Projectile weapon: where the bullets leave on a normal shot (the player rifle). The aim still
	 * comes from the eyes. ViewScale: 1 = the real socket; less = pulled toward the eyes, to match
	 * a first person weapon drawn with the camera First Person Scale.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shoot")
	void SetMuzzle(USceneComponent* InMuzzleComponent, FName InMuzzleSocketName, float InViewScale = 1.f);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ActivateAbility() override;
	virtual void HandleRelease() override;
	virtual void OnAbilityEnd(bool bWasCancelled) override;

private:
	void FireShot();
	void FireShotFrom(const TOptional<FVector>& MuzzleLocation);
	void SpawnProjectile(const FVector& SpawnLocation, const FVector& AimPoint);
	void GetAimViewPoint(FVector& OutLocation, FRotator& OutRotation) const;
	void PlayFireSound(const FVector& Location) const;

	double LastShotTime = -1000.0;
	FTimerHandle FireTimer;

	TWeakObjectPtr<USceneComponent> MuzzleComponent;
	FName MuzzleSocketName;
	float MuzzleViewScale = 1.f;
};
