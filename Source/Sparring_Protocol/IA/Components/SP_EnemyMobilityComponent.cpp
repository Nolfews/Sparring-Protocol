#include "IA/Components/SP_EnemyMobilityComponent.h"
#include "Abilities/SP_DashComponent.h"
#include "Abilities/SP_JumpComponent.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

USP_EnemyMobilityComponent::USP_EnemyMobilityComponent()
{
	// Only ticks during a climb: the service handles the decisions the rest of the time.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void USP_EnemyMobilityComponent::SetCapabilities(int32 NewMaxJumps, bool bNewCanDash, bool bNewCanVault)
{
	MaxJumps = FMath::Max(NewMaxJumps, 0);
	bCanDash = bNewCanDash;
	bCanVault = bNewCanVault;

	if (USP_JumpComponent* Jump = GetOwner()->FindComponentByClass<USP_JumpComponent>())
	{
		Jump->MaxJumps = FMath::Max(MaxJumps, 1);
		Jump->ApplySettings();
	}
}

void USP_EnemyMobilityComponent::UpdateMobility(const FVector& GoalLocation, bool bGoalIsGrounded)
{
	CurrentGoal = GoalLocation;

	if (bClimbing)
	{
		return;
	}

	bool bVault = false;
	if (bGoalIsGrounded && ShouldClimbTo(GoalLocation, bVault))
	{
		StartClimbing(bVault);
		return;
	}

	TryDash(GoalLocation);
}

bool USP_EnemyMobilityComponent::ShouldClimbTo(const FVector& Goal, bool& bOutVault)
{
	bOutVault = false;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;

	if (!Movement || MaxJumps <= 0 || !Movement->IsMovingOnGround() || GetWorld()->GetTimeSeconds() < NextClimbTime)
	{
		return false;
	}

	const FVector Location = Character->GetActorLocation();
	const float HeightDifference = Goal.Z - Location.Z;
	const float Distance2D = FVector::Dist2D(Location, Goal);

	if (HeightDifference > GetMaxClimbHeight() || Distance2D > ClimbRange)
	{
		return false;
	}

	const UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(this, Location, Goal, Character);
	const bool bHasCompletePath = Path && Path->IsValid() && !Path->IsPartial();

	// Another level: climb or drop, unless a ramp leads there.
	if (FMath::Abs(HeightDifference) >= MinHeightDifference)
	{
		return !bHasCompletePath;
	}

	// Same level: only the X-ray tier jumps over obstacles instead of walking around them.
	if (!bCanVault || Distance2D < ArriveDistance)
	{
		return false;
	}

	const bool bLongDetour = bHasCompletePath && Path->GetPathLength() > Distance2D * VaultDetourRatio;
	bOutVault = !bHasCompletePath || bLongDetour;
	return bOutVault;
}

float USP_EnemyMobilityComponent::GetMaxClimbHeight() const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement || MaxJumps <= 0)
	{
		return 0.f;
	}

	const float Gravity = FMath::Abs(Movement->GetGravityZ());
	if (Gravity <= 0.f)
	{
		return 0.f;
	}

	// Height of one jump: velocity squared / (2 * gravity).
	const float JumpHeight = FMath::Square(Movement->JumpZVelocity) / (2.f * Gravity);

	return JumpHeight * MaxJumps * ClimbHeightMargin;
}

void USP_EnemyMobilityComponent::StartClimbing(bool bVault)
{
	bClimbing = true;
	bVaulting = bVault;
	bHasLeftGround = false;
	ClimbEndTime = GetWorld()->GetTimeSeconds() + MaxClimbTime;

	SetComponentTickEnabled(true);
}

void USP_EnemyMobilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;

	if (!Movement)
	{
		StopClimbing();
		return;
	}

	const double Now = GetWorld()->GetTimeSeconds();

	if (bJumpHeld && Now >= JumpReleaseTime)
	{
		ReleaseJump();
	}

	if (!Movement->IsMovingOnGround())
	{
		bHasLeftGround = true;
	}

	const FVector Location = Character->GetActorLocation();
	const float HeightDifference = CurrentGoal.Z - Location.Z;
	const bool bSameLevel = FMath::Abs(HeightDifference) < MinHeightDifference;
	const bool bCloseEnough = FVector::Dist2D(Location, CurrentGoal) < ArriveDistance;

	// A vault starts and ends at the same level: it is over once the enemy has left the ground and landed.
	const bool bArrived = Movement->IsMovingOnGround() && bSameLevel && (!bVaulting || bHasLeftGround || bCloseEnough);

	if (bArrived || Now >= ClimbEndTime)
	{
		StopClimbing();
		NextClimbTime = Now + ClimbRetryDelay;
		return;
	}

	// The Behavior Tree can start a new Move To: the climb keeps the priority.
	if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
	{
		if (AIController->GetMoveStatus() != EPathFollowingStatus::Idle)
		{
			AIController->StopMovement();
		}
	}

	const FVector Direction = (CurrentGoal - Location).GetSafeNormal2D();
	Character->AddMovementInput(Direction);

	if (Movement->IsMovingOnGround())
	{
		if ((HeightDifference > 0.f || bVaulting) && IsWallAhead(Direction, 0.f))
		{
			// A ray at the highest point the jumps can reach: if it hits, the wall is too tall.
			const float FeetToCenter = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			if (IsWallAhead(Direction, GetMaxClimbHeight() - FeetToCenter))
			{
				// Let the Move To walk around it.
				StopClimbing();
				NextClimbTime = Now + ClimbRetryDelay;
				return;
			}

			TryJump();
		}
	}
	else if ((HeightDifference > 0.f || (bVaulting && IsWallAhead(Direction, 0.f)))
		&& Movement->Velocity.Z <= NextJumpMaxVelocityZ
		&& Character->JumpCurrentCount < MaxJumps)
	{
		TryJump();
	}
}

bool USP_EnemyMobilityComponent::IsWallAhead(const FVector& Direction, float HeightOffset) const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const FVector Start = Character->GetActorLocation() + FVector(0.f, 0.f, HeightOffset);
	const float Distance = Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + WallCheckDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	// Same channel as the AI sight.
	return GetWorld()->LineTraceTestByChannel(Start, Start + Direction * Distance, ECC_Visibility, Params);
}

void USP_EnemyMobilityComponent::TryJump()
{
	const double Now = GetWorld()->GetTimeSeconds();
	if (bJumpHeld || Now < NextJumpTime)
	{
		return;
	}

	USP_JumpComponent* Jump = GetOwner()->FindComponentByClass<USP_JumpComponent>();
	if (Jump && Jump->TryActivate())
	{
		bJumpHeld = true;
		JumpReleaseTime = Now + JumpPressTime;
		NextJumpTime = Now + JumpRetryDelay;
	}
}

void USP_EnemyMobilityComponent::ReleaseJump()
{
	if (!bJumpHeld)
	{
		return;
	}

	bJumpHeld = false;
	if (USP_JumpComponent* Jump = GetOwner()->FindComponentByClass<USP_JumpComponent>())
	{
		Jump->Release();
	}
}

void USP_EnemyMobilityComponent::StopClimbing()
{
	ReleaseJump();
	bClimbing = false;
	bVaulting = false;
	SetComponentTickEnabled(false);
}

void USP_EnemyMobilityComponent::TryDash(const FVector& Goal)
{
	const double Now = GetWorld()->GetTimeSeconds();
	if (!bCanDash || Now < NextDashTime)
	{
		return;
	}

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement || !Movement->IsMovingOnGround() || Movement->Velocity.SizeSquared2D() < FMath::Square(DashMinSpeed))
	{
		return;
	}

	const FVector Location = Character->GetActorLocation();
	const float Distance = FVector::Dist2D(Location, Goal);
	if (Distance < DashMinDistance || Distance > DashMaxDistance)
	{
		return;
	}

	// USP_DashComponent follows the velocity: only dash when it leads to the target.
	const FVector MoveDirection = Movement->Velocity.GetSafeNormal2D();
	const FVector GoalDirection = (Goal - Location).GetSafeNormal2D();
	if (FVector::DotProduct(MoveDirection, GoalDirection) < 0.5f)
	{
		return;
	}

	USP_DashComponent* Dash = GetOwner()->FindComponentByClass<USP_DashComponent>();
	if (Dash && Dash->TryActivate())
	{
		NextDashTime = Now + DashInterval;
	}
}