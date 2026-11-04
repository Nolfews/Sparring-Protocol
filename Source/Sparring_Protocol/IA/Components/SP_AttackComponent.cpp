#include "IA/Components/SP_AttackComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "DrawDebugHelpers.h"

USP_AttackComponent::USP_AttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool USP_AttackComponent::StartAttack()
{
	if (bIsAttacking || !AttackMontage)
	{
		return false;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UAnimInstance* AnimInstance = (Character && Character->GetMesh()) ? Character->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		return false;
	}

	// 0 means the montage could not play (no matching Slot in the AnimGraph, wrong skeleton...).
	if (Character->PlayAnimMontage(AttackMontage) <= 0.f)
	{
		return false;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &USP_AttackComponent::HandleMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);

	bIsAttacking = true;
	return true;
}

void USP_AttackComponent::StopAttack()
{
	if (!bIsAttacking)
	{
		return;
	}

	// Triggers HandleMontageEnded (bInterrupted = true), which resets the state.
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->StopAnimMontage(AttackMontage);
	}
}

void USP_AttackComponent::NotifyAttackFired()
{
#if ENABLE_DRAW_DEBUG
	if (bDrawDebugAttack && GetOwner())
	{
		FVector EyesLocation;
		FRotator EyesRotation;
		GetOwner()->GetActorEyesViewPoint(EyesLocation, EyesRotation);
		DrawDebugLine(GetWorld(), EyesLocation, EyesLocation + EyesRotation.Vector() * 1000.f, FColor::Red, false, 1.f, 0, 2.f);
	}
#endif

	OnAttackFired.Broadcast();
}

void USP_AttackComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
}
