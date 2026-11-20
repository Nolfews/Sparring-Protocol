#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_ShootWhileMoving.generated.h"

UCLASS()
class SPARRING_PROTOCOL_API UBTService_ShootWhileMoving : public UBTService_BlackboardBase
{
    GENERATED_BODY()

    public:
        UBTService_ShootWhileMoving();

    protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

};