#include "Abilities/SP_SlideComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USP_SlideComponent::USP_SlideComponent()
{
	// Ticks only while sliding, before the movement so our velocity is used this frame.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void USP_SlideComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* Movement = GetMovement())
	{
		Movement->GetNavAgentPropertiesRef().bCanCrouch = true;
		Movement->SetCrouchedHalfHeight(SlideCapsuleHalfHeight);
		Movement->AddTickPrerequisiteComponent(this);
	}
}

bool USP_SlideComponent::CanActivateAbility() const
{
	const ACharacter* Character = GetCharacter();
	const UCharacterMovementComponent* Movement = GetMovement();

	return Movement
		&& Movement->IsMovingOnGround()
		&& !Character->bIsCrouched
		&& Character->CanCrouch()
		&& Movement->Velocity.Size2D() >= MinStartSpeed;
}

void USP_SlideComponent::ActivateAbility()
{
	UCharacterMovementComponent* Movement = GetMovement();
	const FVector GroundVelocity(Movement->Velocity.X, Movement->Velocity.Y, 0.f);

	SlideDirection = GroundVelocity.GetSafeNormal();
	CurrentSpeed = FMath::Max(SlideSpeed, GroundVelocity.Size());
	ElapsedTime = 0.f;

	SavedGroundFriction = Movement->GroundFriction;
	SavedBrakingDeceleration = Movement->BrakingDecelerationWalking;
	SavedMaxCrouchSpeed = Movement->MaxWalkSpeedCrouched;

	Movement->GroundFriction = 0.f;
	Movement->BrakingDecelerationWalking = 0.f;
	Movement->MaxWalkSpeedCrouched = CurrentSpeed;

	GetCharacter()->Crouch();
	SetComponentTickEnabled(true);
}

void USP_SlideComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UCharacterMovementComponent* Movement = GetMovement();
	if (!IsAbilityActive() || !Movement)
	{
		return;
	}

	// Left the ground (jump, ledge): stop sliding but keep the speed.
	if (!Movement->IsMovingOnGround())
	{
		EndAbility();
		return;
	}

	// Hit a wall: the movement removed most of our speed last frame.
	if (ElapsedTime > 0.1f && Movement->Velocity.Size2D() < CurrentSpeed * 0.3f)
	{
		EndAbility();
		return;
	}

	ElapsedTime += DeltaTime;
	CurrentSpeed = FMath::Max(MinSlideSpeed, CurrentSpeed - Deceleration * DeltaTime);

	Movement->MaxWalkSpeedCrouched = CurrentSpeed;
	const float VerticalSpeed = Movement->Velocity.Z;
	Movement->Velocity = SlideDirection * CurrentSpeed;
	Movement->Velocity.Z = VerticalSpeed;

	if (MaxDuration > 0.f && ElapsedTime >= MaxDuration)
	{
		EndAbility();
	}
}

void USP_SlideComponent::HandleRelease()
{
	EndAbility();
}

void USP_SlideComponent::OnAbilityEnd(bool bWasCancelled)
{
	SetComponentTickEnabled(false);

	if (UCharacterMovementComponent* Movement = GetMovement())
	{
		Movement->GroundFriction = SavedGroundFriction;
		Movement->BrakingDecelerationWalking = SavedBrakingDeceleration;
		Movement->MaxWalkSpeedCrouched = SavedMaxCrouchSpeed;
	}

	if (ACharacter* Character = GetCharacter())
	{
		Character->UnCrouch();
	}
}
