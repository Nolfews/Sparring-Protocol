#include "IA/BehaviorTree/BTTask_FindPatrolLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindPatrolLocation::UBTTask_FindPatrolLocation()
{
	NodeName = TEXT("Find Patrol Location");

	// Only Vector keys appear in the dropdown of the editor.
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindPatrolLocation, BlackboardKey));
}

EBTNodeResult::Type UBTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!Pawn || !BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}

	FNavLocation Result;
	if (!NavSys->GetRandomReachablePointInRadius(Pawn->GetActorLocation(), SearchRadius, Result))
	{
		return EBTNodeResult::Failed;
	}

	BlackboardComp->SetValueAsVector(BlackboardKey.SelectedKeyName, Result.Location);
	return EBTNodeResult::Succeeded;
}
