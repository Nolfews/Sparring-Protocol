#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_LookAround.generated.h"

/** Progress of one turn, stored per AI (a task node is shared by every enemy using the tree). */
struct FBTLookAroundMemory
{
	float StartYaw = 0.f;
	float TurnedAngle = 0.f;
	float TurnDirection = 1.f;
};

/**
 * The enemy turns on itself to look around. The key is a bool (bCanLookAround):
 * while it is false, the task succeeds at once and the patrol goes on as before.
 */
UCLASS()
class SPARRING_PROTOCOL_API UBTTask_LookAround : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_LookAround();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTLookAroundMemory); }
	virtual FString GetStaticDescription() const override;

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	/** 360 = a full turn. */
	UPROPERTY(EditAnywhere, Category = "Look Around", meta = (ClampMin = "10", ClampMax = "720", Units = "Degrees"))
	float TurnAngle = 360.f;

	/** Keep it below the Rotation Rate of the character (360) so the body keeps up with the eyes. */
	UPROPERTY(EditAnywhere, Category = "Look Around", meta = (ClampMin = "10", Units = "DegreesPerSecond"))
	float TurnSpeed = 120.f;
};