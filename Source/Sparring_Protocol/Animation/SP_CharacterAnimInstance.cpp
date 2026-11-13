#include "Animation/SP_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "IA/Components/SP_AttackComponent.h"

void USP_CharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		OwnerMovement = OwnerCharacter->GetCharacterMovement();
		OwnerAttack = OwnerCharacter->FindComponentByClass<USP_AttackComponent>();
	}
}

void USP_CharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// No owner in the editor preview: keep the defaults.
	if (!OwnerCharacter || !OwnerMovement)
	{
		return;
	}

	const FVector Velocity = OwnerMovement->Velocity;
	GroundSpeed = Velocity.Size2D();
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());
	bIsFalling = OwnerMovement->IsFalling();

	// The blend space stops at its fastest sample: past it, speed the animation up instead.
	LocomotionPlayRate = FMath::Clamp(GroundSpeed / BlendSpaceMaxSpeed, 1.f, MaxLocomotionPlayRate);

	const float TargetAim = (OwnerAttack && OwnerAttack->WantsToAim()) ? 1.f : 0.f;
	AimAlpha = FMath::FInterpTo(AimAlpha, TargetAim, DeltaSeconds, AimInterpSpeed);
}