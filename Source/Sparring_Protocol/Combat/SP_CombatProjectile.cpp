#include "Combat/SP_CombatProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

ASP_CombatProjectile::ASP_CombatProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(5.f);
	Collision->SetCollisionProfileName(TEXT("Projectile")); // preset already in the project
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->UpdatedComponent = Collision;
	Movement->ProjectileGravityScale = 0.f; // straight line
	Movement->bRotationFollowsVelocity = true;
	Movement->bShouldBounce = false;
	Movement->InitialSpeed = 0.f; // the speed comes from Launch()
	Movement->MaxSpeed = 0.f;     // 0 = no limit
}

void ASP_CombatProjectile::BeginPlay()
{
	Super::BeginPlay();

	// The "Projectile" preset blocks every channel, Visibility included. The AI sight, the aim trace and
	// the obstacle checks all trace on Visibility: a bullet in flight between the enemy and the player
	// made the enemy lose sight of him (and stop shooting when it cannot search yet).
	// Done here and not in the constructor so a Blueprint collision override cannot bring it back.
	Collision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Fired by the movement component on the first blocking hit.
	Movement->OnProjectileStop.AddDynamic(this, &ASP_CombatProjectile::HandleStop);
}

void ASP_CombatProjectile::Launch(const FVector& Direction, float Speed, float InDamage, TSubclassOf<UDamageType> InDamageType)
{
	Damage = InDamage;
	DamageType = InDamageType;

	// Never hit the shooter (the bullet starts inside its capsule).
	if (APawn* Shooter = GetInstigator())
	{
		Collision->IgnoreActorWhenMoving(Shooter, true);
	}

	Movement->Velocity = Direction * Speed;
	SetLifeSpan(LifeTime);
}

void ASP_CombatProjectile::HandleStop(const FHitResult& ImpactResult)
{
	if (AActor* HitActor = ImpactResult.GetActor())
	{
		// The shooter is the damage causer, like the hitscan: the health component reports it as the killer.
		APawn* Shooter = GetInstigator();
		UGameplayStatics::ApplyPointDamage(HitActor, Damage, GetActorForwardVector(), ImpactResult,
			Shooter ? Shooter->GetController() : nullptr, Shooter, DamageType);
	}

	SpawnImpactEffect(ImpactResult);
	Destroy();
}

void ASP_CombatProjectile::SpawnImpactEffect(const FHitResult& ImpactResult) const
{
	if (!ImpactEffect)
	{
		return;
	}

	// The effect lives in the world, not on the bullet (destroyed right after).
	// Its up axis follows the surface: the sparks fly out of the wall, like out of the floor in the pack demo.
	const FRotator Rotation = FRotationMatrix::MakeFromZ(ImpactResult.ImpactNormal).Rotator();
	UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this, ImpactEffect, ImpactResult.ImpactPoint, Rotation, FVector(ImpactEffectScale));

	if (!Effect || ImpactEffectDuration <= 0.f)
	{
		return;
	}

	// A looping system would emit forever: stop it after a short burst. It destroys itself
	// once its last particle dies (auto destroy). Weak lambda = nothing happens if it is already gone.
	FTimerHandle StopTimer;
	GetWorldTimerManager().SetTimer(StopTimer,
		FTimerDelegate::CreateWeakLambda(Effect, [Effect]() { Effect->Deactivate(); }),
		ImpactEffectDuration, false);
}