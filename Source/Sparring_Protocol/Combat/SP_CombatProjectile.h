#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SP_CombatProjectile.generated.h"

class UDamageType;
class UNiagaraSystem;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

/** A flying bullet: damages the first thing it hits, then disappears. */
UCLASS()
class SPARRING_PROTOCOL_API ASP_CombatProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASP_CombatProjectile();

	/** Called by the shoot component right after the spawn. */
	void Launch(const FVector& Direction, float Speed, float InDamage, TSubclassOf<UDamageType> InDamageType);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> Collision;

	/** Visual only: the collision is the sphere. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> Movement;

	/** Destroyed after this time if it hits nothing. */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (ClampMin = "0.1", Units = "s"))
	float LifeTime = 3.f;

	/** Played where the bullet hits (walls, floor, characters). Empty = no effect. */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Impact")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	/**
	 * The effect stops emitting after this time, the live particles finish their life.
	 * Needed for looping systems (most spark packs loop). 0 = the system ends by itself.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Impact", meta = (ClampMin = "0", Units = "s"))
	float ImpactEffectDuration = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Impact", meta = (ClampMin = "0.01"))
	float ImpactEffectScale = 1.f;

private:
	UFUNCTION()
	void HandleStop(const FHitResult& ImpactResult);

	void SpawnImpactEffect(const FHitResult& ImpactResult) const;

	float Damage = 0.f;

	UPROPERTY()
	TSubclassOf<UDamageType> DamageType;
};