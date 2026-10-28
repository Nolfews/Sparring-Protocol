#pragma once

#include "CoreMinimal.h"
#include "Abilities/SP_AbilityComponent.h"
#include "Engine/EngineTypes.h"
#include "SP_ShootComponent.generated.h"

class UDamageType;

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

	/** Draws the impacts (red = hit something, green = nothing). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Debug")
	bool bDrawDebug = true;

	UPROPERTY(BlueprintAssignable, Category = "Shoot|Events")
	FSPOnShotFired OnShotFired;

	UPROPERTY(BlueprintAssignable, Category = "Shoot|Events")
	FSPOnShotTrace OnShotTrace;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ActivateAbility() override;
	virtual void HandleRelease() override;
	virtual void OnAbilityEnd(bool bWasCancelled) override;

private:
	void FireShot();
	void GetAimViewPoint(FVector& OutLocation, FRotator& OutRotation) const;

	double LastShotTime = -1000.0;
	FTimerHandle FireTimer;
};
