#include "IA/BehaviorTree/BTTask_SetMovementPace.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_SetMovementPace::UBTTask_SetMovementPace()
{
	NodeName = TEXT("Set Movement Pace");
}

EBTNodeResult::Type UBTTask_SetMovementPace::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	USP_MovementPaceComponent* PaceComponent = ControlledPawn ? ControlledPawn->FindComponentByClass<USP_MovementPaceComponent>() : nullptr;
	if (!PaceComponent)
	{
		return EBTNodeResult::Failed;
	}

	PaceComponent->SetPace(Pace);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_SetMovementPace::GetStaticDescription() const
{
	return FString::Printf(TEXT("Pace: %s"), *UEnum::GetDisplayValueAsText(Pace).ToString());
}
