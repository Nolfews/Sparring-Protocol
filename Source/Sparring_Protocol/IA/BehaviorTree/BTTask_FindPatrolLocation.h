#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindPatrolLocation.generated.h"

/** Writes a random point reachable on the NavMesh around the pawn into a Vector key. */
UCLASS()
class SPARRING_PROTOCOL_API UBTTask_FindPatrolLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_FindPatrolLocation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	/** Radius around the pawn in which a reachable point is picked, in cm. */
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "100"))
	float SearchRadius = 1500.f;
};
