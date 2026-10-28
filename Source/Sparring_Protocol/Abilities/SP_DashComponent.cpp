#include "Abilities/SP_DashComponent.h"

#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

void USP_DashComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentCharges = MaxCharges;
}

void USP_DashComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DashTimer);
		World->GetTimerManager().ClearTimer(RefillTimer);
	}
	Super::EndPlay(EndPlayReason);
}

bool USP_DashComponent::CanActivateAbility() const
{
	return CurrentCharges > 0 && GetMovement() != nullptr;
}

FVector USP_DashComponent::GetDashDirection() const
{
	const ACharacter* Character = GetCharacter();

	// Player: the movement keys held this frame (or the last frame).
	FVector Direction = Character->GetPendingMovementInputVector();
	if (Direction.IsNearlyZero())
	{
		Direction = Character->GetLastMovementInputVector();
	}
	// AI (no input vector): its current velocity.
	if (Direction.IsNearlyZero())
	{
		Direction = GetMovement()->Velocity;
	}
	Direction.Z = 0.f;

	// Not moving at all: dash forward.
	if (Direction.IsNearlyZero())
	{
		Direction = Character->GetActorForwardVector();
		Direction.Z = 0.f;
	}

	return Direction.GetSafeNormal();
}

void USP_DashComponent::ActivateAbility()
{
	ACharacter* Character = GetCharacter();
	UCharacterMovementComponent* Movement = GetMovement();
	UWorld* World = GetWorld();

	--CurrentCharges;
	if (!World->GetTimerManager().IsTimerActive(RefillTimer))
	{
		World->GetTimerManager().SetTimer(RefillTimer, this, &USP_DashComponent::RefillCharge, ChargeRefillTime, true);
	}

	if (Character->bIsCrouched)
	{
		Character->UnCrouch();
	}

	SavedGravityScale = Movement->GravityScale;
	if (bIgnoreGravity)
	{
		Movement->GravityScale = 0.f;
	}

	// Launch switches to "falling" mode, so ground friction does not eat the dash.
	Character->LaunchCharacter(GetDashDirection() * DashSpeed, true, true);

	World->GetTimerManager().SetTimer(DashTimer, this, &USP_DashComponent::FinishDash, DashDuration, false);
}

void USP_DashComponent::FinishDash()
{
	EndAbility();
}

void USP_DashComponent::OnAbilityEnd(bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DashTimer);
	}

	UCharacterMovementComponent* Movement = GetMovement();
	if (!Movement)
	{
		return;
	}

	Movement->GravityScale = SavedGravityScale;

	const FVector Horizontal(Movement->Velocity.X, Movement->Velocity.Y, 0.f);
	if (Horizontal.Size() > SpeedAfterDash)
	{
		const FVector Clamped = Horizontal.GetSafeNormal() * SpeedAfterDash;
		Movement->Velocity = FVector(Clamped.X, Clamped.Y, Movement->Velocity.Z);
	}
}

void USP_DashComponent::RefillCharge()
{
	CurrentCharges = FMath::Min(CurrentCharges + 1, MaxCharges);
	if (CurrentCharges >= MaxCharges)
	{
		GetWorld()->GetTimerManager().ClearTimer(RefillTimer);
	}
}
