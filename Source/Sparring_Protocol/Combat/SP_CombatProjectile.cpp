#include "Combat/SP_CombatProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

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

	Destroy();
}