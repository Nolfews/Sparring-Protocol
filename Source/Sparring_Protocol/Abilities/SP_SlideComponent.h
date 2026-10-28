#pragma once

#include "CoreMinimal.h"
#include "Abilities/SP_AbilityComponent.h"
#include "SP_SlideComponent.generated.h"

/**
 * Hold to slide on the ground in a fixed direction, with a lower capsule.
 * The slide stops when the button is released, when the character leaves the ground
 * (jumping keeps the speed: slide jump), when it hits a wall, or after MaxDuration.
 * Deceleration = 0 gives a constant speed slide (Ultrakill style).
 */
UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API USP_SlideComponent : public USP_AbilityComponent
{
	GENERATED_BODY()

public:
	USP_SlideComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** The character must move at least this fast to start a slide. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "0", Units = "CentimetersPerSecond"))
	float MinStartSpeed = 200.f;

	/** Speed at the start of the slide (the current speed is kept if it is higher). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "0", Units = "CentimetersPerSecond"))
	float SlideSpeed = 1400.f;

	/** The speed goes down to this value and stays there. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "0", Units = "CentimetersPerSecond"))
	float MinSlideSpeed = 700.f;

	/** Speed lost per second. 0 = constant speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "0"))
	float Deceleration = 500.f;

	/** Maximum duration of a slide. 0 = as long as the button is held. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "0", Units = "s"))
	float MaxDuration = 0.f;

	/** Capsule half height while sliding (standing is 96 on the player). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slide", meta = (ClampMin = "10", Units = "cm"))
	float SlideCapsuleHalfHeight = 50.f;

protected:
	virtual void BeginPlay() override;
	virtual bool CanActivateAbility() const override;
	virtual void ActivateAbility() override;
	virtual void HandleRelease() override;
	virtual void OnAbilityEnd(bool bWasCancelled) override;

private:
	FVector SlideDirection = FVector::ForwardVector;
	float CurrentSpeed = 0.f;
	float ElapsedTime = 0.f;

	// Movement values changed during the slide, restored at the end.
	float SavedGroundFriction = 0.f;
	float SavedBrakingDeceleration = 0.f;
	float SavedMaxCrouchSpeed = 0.f;
};
