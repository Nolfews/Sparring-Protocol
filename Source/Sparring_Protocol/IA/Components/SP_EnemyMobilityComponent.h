#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SP_EnemyMobilityComponent.generated.h"

/**
 * Decides when the enemy jumps and dashes. The Behavior Tree gives the goal,
 * the jump and dash abilities perform the moves.
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API USP_EnemyMobilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USP_EnemyMobilityComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 0 jumps = the enemy stays on the navmesh. */
	void SetCapabilities(int32 NewMaxJumps, bool bNewCanDash);

	/** Called by the Behavior Tree service with the position of the target. */
	void UpdateMobility(const FVector& GoalLocation, bool bGoalIsGrounded);

	void StopClimbing();

protected:
	/** Smaller height differences are left to the normal movement. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Climb", meta = (ClampMin = "0"))
	float MinHeightDifference = 120.f;

	/** The enemy only climbs when the target is this close horizontally. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Climb", meta = (ClampMin = "0"))
	float ClimbRange = 1200.f;

	/** 0.85 = the enemy only tries 85% of its theoretical jump height. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Climb", meta = (ClampMin = "0.1", ClampMax = "1"))
	float ClimbHeightMargin = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Climb", meta = (ClampMin = "0"))
	float WallCheckDistance = 120.f;

	/** The enemy gives up after this time. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Climb", meta = (ClampMin = "0.1"))
	float MaxClimbTime = 3.f;

	/** Pause before the next climb. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Climb", meta = (ClampMin = "0"))
	float ClimbRetryDelay = 2.f;

	/** Minimum time between two jumps. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Climb", meta = (ClampMin = "0"))
	float JumpRetryDelay = 0.6f;

	/** How long the jump stays pressed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Climb", meta = (ClampMin = "0.01"))
	float JumpPressTime = 0.1f;

	/** The next jump starts once the vertical speed falls below this value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Climb")
	float NextJumpMaxVelocityZ = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Dash", meta = (ClampMin = "0"))
	float DashMinDistance = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Dash", meta = (ClampMin = "0"))
	float DashMaxDistance = 2500.f;

	/** Seconds between two dashes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Dash", meta = (ClampMin = "0"))
	float DashInterval = 3.f;

	/** The AI dashes along its velocity: it must already be moving. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Dash", meta = (ClampMin = "0"))
	float DashMinSpeed = 100.f;

private:
	bool ShouldClimbTo(const FVector& Goal);
	float GetMaxClimbHeight() const;
	bool IsWallAhead(const FVector& Direction) const;
	void StartClimbing();
	void TryJump();
	void ReleaseJump();
	void TryDash(const FVector& Goal);

	int32 MaxJumps = 0;
	bool bCanDash = false;
	bool bClimbing = false;
	bool bJumpHeld = false;
	FVector CurrentGoal = FVector::ZeroVector;

	double ClimbEndTime = 0.0;
	double NextClimbTime = 0.0;
	double NextJumpTime = 0.0;
	double JumpReleaseTime = 0.0;
	double NextDashTime = 0.0;
};