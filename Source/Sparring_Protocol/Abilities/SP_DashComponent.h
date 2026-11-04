#pragma once

#include "CoreMinimal.h"
#include "Abilities/SP_AbilityComponent.h"
#include "SP_DashComponent.generated.h"

/**
 * Short burst of speed in the movement direction (forward if the character is not moving).
 * Works on the ground and in the air. Uses charges that refill over time (Ultrakill stamina).
 */
UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API USP_DashComponent : public USP_AbilityComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0", Units = "CentimetersPerSecond"))
	float DashSpeed = 2800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0.01", Units = "s"))
	float DashDuration = 0.15f;

	/** Horizontal speed is capped to this value when the dash ends. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0", Units = "CentimetersPerSecond"))
	float SpeedAfterDash = 1000.f;

	/** No gravity during the dash: it stays horizontal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bIgnoreGravity = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Charges", meta = (ClampMin = "1"))
	int32 MaxCharges = 3;

	/** Time to get one charge back. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash|Charges", meta = (ClampMin = "0.01", Units = "s"))
	float ChargeRefillTime = 1.f;

	UFUNCTION(BlueprintPure, Category = "Dash")
	int32 GetCharges() const { return CurrentCharges; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool CanActivateAbility() const override;
	virtual void ActivateAbility() override;
	virtual void OnAbilityEnd(bool bWasCancelled) override;

private:
	void FinishDash();
	void RefillCharge();
	FVector GetDashDirection() const;

	int32 CurrentCharges = 0;
	float SavedGravityScale = 1.f;
	FTimerHandle DashTimer;
	FTimerHandle RefillTimer;
};
