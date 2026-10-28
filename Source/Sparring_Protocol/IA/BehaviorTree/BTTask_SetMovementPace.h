#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "IA/Components/SP_MovementPaceComponent.h"
#include "BTTask_SetMovementPace.generated.h"

/** Switches the pawn's USP_MovementPaceComponent to walk or run. */
UCLASS()
class SPARRING_PROTOCOL_API UBTTask_SetMovementPace : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetMovementPace();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Movement")
	ESP_MovementPace Pace = ESP_MovementPace::Walk;
};
