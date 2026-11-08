#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SP_CombatProjectile.generated.h"

class UDamageType;
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

private:
	UFUNCTION()
	void HandleStop(const FHitResult& ImpactResult);

	float Damage = 0.f;

	UPROPERTY()
	TSubclassOf<UDamageType> DamageType;
};