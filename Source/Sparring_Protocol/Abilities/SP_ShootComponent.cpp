#include "Abilities/SP_ShootComponent.h"

#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void USP_ShootComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimer);
	}
	Super::EndPlay(EndPlayReason);
}

void USP_ShootComponent::ActivateAbility()
{
	FTimerManager& Timers = GetWorld()->GetTimerManager();
	const float Interval = 1.f / FireRate;

	// Respect the fire rate even if the player spams the button.
	const float Remaining = FMath::Max(0.f, static_cast<float>(LastShotTime + Interval - GetWorldTime()));

	if (bAutomatic)
	{
		if (Remaining <= 0.f)
		{
			FireShot();
			Timers.SetTimer(FireTimer, this, &USP_ShootComponent::FireShot, Interval, true);
		}
		else
		{
			Timers.SetTimer(FireTimer, this, &USP_ShootComponent::FireShot, Interval, true, Remaining);
		}
	}
	else
	{
		if (Remaining <= 0.f)
		{
			FireShot();
		}
		else
		{
			// Clicked a bit too early: the shot is buffered and fires as soon as possible.
			Timers.SetTimer(FireTimer, this, &USP_ShootComponent::FireShot, Remaining, false);
		}
	}
}

void USP_ShootComponent::HandleRelease()
{
	EndAbility();
}

void USP_ShootComponent::OnAbilityEnd(bool bWasCancelled)
{
	// Semi-automatic: a buffered shot still fires after the release, unless cancelled.
	if (bAutomatic || bWasCancelled)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(FireTimer);
		}
	}
}

void USP_ShootComponent::GetAimViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	const ACharacter* Character = GetCharacter();

	if (const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		// Player: exactly what the camera sees, so the shot goes under the crosshair.
		PlayerController->GetPlayerViewPoint(OutLocation, OutRotation);
	}
	else
	{
		// AI: eye height and the rotation set by the AI controller (its focus).
		Character->GetActorEyesViewPoint(OutLocation, OutRotation);
	}
}

void USP_ShootComponent::FireShot()
{
	ACharacter* Character = GetCharacter();
	UWorld* World = GetWorld();
	if (!Character || !World || !bAbilityEnabled)
	{
		return;
	}

	LastShotTime = GetWorldTime();

	FVector Start;
	FRotator AimRotation;
	GetAimViewPoint(Start, AimRotation);
	const FVector AimDirection = AimRotation.Vector();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SPShoot), true, Character);
	Params.AddIgnoredActor(Character);

	OnShotFired.Broadcast();

	const float HalfConeRadians = FMath::DegreesToRadians(SpreadAngle * 0.5f);

	for (int32 Pellet = 0; Pellet < PelletsPerShot; ++Pellet)
	{
		const FVector Direction = HalfConeRadians > 0.f ? FMath::VRandCone(AimDirection, HalfConeRadians) : AimDirection;
		const FVector End = Start + Direction * Range;

		FHitResult Hit;
		const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params);
		const FVector TraceEnd = bHit ? Hit.ImpactPoint : End;

		if (bHit && Hit.GetActor())
		{
			UGameplayStatics::ApplyPointDamage(Hit.GetActor(), Damage, Direction, Hit, Character->GetController(), Character, DamageType);
		}

		if (bDrawDebug)
		{
			DrawDebugLine(World, Start, TraceEnd, bHit ? FColor::Red : FColor::Green, false, 1.f, 0, 0.5f);
			if (bHit)
			{
				DrawDebugSphere(World, Hit.ImpactPoint, 6.f, 8, FColor::Red, false, 1.f);
			}
		}

		OnShotTrace.Broadcast(Start, TraceEnd, Hit);
	}
}
