#include "IA/BehaviorTree/BTTask_Attack.h"
#include "IA/Components/SP_AttackComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

namespace
{
	// The task is shared by every AI using the tree: never cache this, look it up on each call.
	USP_AttackComponent* FindAttackComponent(const UBehaviorTreeComponent& OwnerComp)
	{
		const AAIController* AIController = OwnerComp.GetAIOwner();
		const APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
		return Pawn ? Pawn->FindComponentByClass<USP_AttackComponent>() : nullptr;
	}
}

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
	bNotifyTick = true; // without this, TickTask is never called
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	USP_AttackComponent* Attack = FindAttackComponent(OwnerComp);
	if (!Attack || !Attack->StartAttack())
	{
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	const USP_AttackComponent* Attack = FindAttackComponent(OwnerComp);
	if (!Attack || !Attack->IsAttacking())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (USP_AttackComponent* Attack = FindAttackComponent(OwnerComp))
	{
		Attack->StopAttack();
	}
	return EBTNodeResult::Aborted;
}
