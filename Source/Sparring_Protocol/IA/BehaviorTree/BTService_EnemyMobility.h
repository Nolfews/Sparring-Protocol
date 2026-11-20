#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_EnemyMobility.generated.h"

/**
 * Gives a goal to the mobility component while its branch is active.
 * The key is an actor (the seen player) or a position (last known or X-ray location).
 */
UCLASS()
class SPARRING_PROTOCOL_API UBTService_EnemyMobility : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_EnemyMobility();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};