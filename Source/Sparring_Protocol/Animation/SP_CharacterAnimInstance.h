#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SP_CharacterAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;

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

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> OwnerMovement;
};
