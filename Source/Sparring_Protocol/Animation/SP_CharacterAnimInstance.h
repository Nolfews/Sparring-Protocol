#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SP_CharacterAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class USP_AttackComponent;

/** Computes the locomotion variables read by an Animation Blueprint. Reads only, never decides. */
UCLASS()
class SPARRING_PROTOCOL_API USP_CharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** Horizontal speed in cm/s. Drives idle / walk / run in the blend space. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float GroundSpeed = 0.f;

	/** Angle between where the body faces and where it moves, in degrees (-180..180). */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsFalling = false;

	/**
	 * Play rate of the locomotion blend space. 1 up to BlendSpaceMaxSpeed, then grows with the speed
	 * so the feet keep up with the ground (700 cm/s with a 500 blend space = 1.4).
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float LocomotionPlayRate = 1.f;

	/** Speed of the fastest animation in the blend space (the jog samples of BS_Rifle_Locomotion). */
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion", meta = (ClampMin = "1"))
	float BlendSpaceMaxSpeed = 500.f;

	/** Limit for very short bursts (dash): beyond this the legs look like a cartoon. */
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion", meta = (ClampMin = "1"))
	float MaxLocomotionPlayRate = 2.f;

	/** 0 = upper body follows the locomotion, 1 = upper body aims. */
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	float AimAlpha = 0.f;

	/** How fast the weapon goes up and down. */
	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.1"))
	float AimInterpSpeed = 12.f;

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> OwnerMovement;

	UPROPERTY(Transient)
	TObjectPtr<USP_AttackComponent> OwnerAttack;
};