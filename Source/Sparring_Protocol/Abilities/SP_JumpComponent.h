#pragma once

#include "CoreMinimal.h"
#include "Abilities/SP_AbilityComponent.h"
#include "SP_JumpComponent.generated.h"

/**
 * Jump, double jump and variable height (hold the button to jump higher).
 * Also works while sliding: the slide is cancelled and its momentum is kept (slide jump).
 *
 * The values are pushed to the CharacterMovement at BeginPlay. If they are changed at
 * runtime (AI getting stronger each round), call ApplySettings().
 */
UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API USP_JumpComponent : public USP_AbilityComponent
{
	GENERATED_BODY()

public:
	/** Vertical speed of a jump. Higher value = higher jump. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (ClampMin = "0", Units = "CentimetersPerSecond"))
	float JumpVelocity = 700.f;

	/** 1 = simple jump, 2 = double jump, 3 = triple jump... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (ClampMin = "1"))
	int32 MaxJumps = 2;

	/** How long the button can be held to jump higher. 0 = every jump has the full height. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (ClampMin = "0", Units = "s"))
	float MaxHoldTime = 0.f;

	/** Control of the direction while in the air (0 = none, 1 = full). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (ClampMin = "0", ClampMax = "1"))
	float AirControl = 0.5f;

	/** Pushes the values above to the Character and its CharacterMovement. */
	UFUNCTION(BlueprintCallable, Category = "Jump")
	void ApplySettings();

protected:
	virtual void BeginPlay() override;
	virtual void ActivateAbility() override;
	virtual void HandleRelease() override;
	virtual void OnAbilityEnd(bool bWasCancelled) override;
};
