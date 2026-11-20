#include "IA/BehaviorTree/BTService_ShootWhileMoving.h"
#include "IA/Components/SP_AttackComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTService_ShootWhileMoving::UBTService_ShootWhileMoving()
{
    NodeName = TEXT("Shoot While Moving");
    Interval = 0.1f;
    RandomDeviation = 0.05f;

   BlackboardKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ShootWhileMoving, BlackboardKey));
}

void UBTService_ShootWhileMoving::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!BlackboardComp || !BlackboardComp->GetValueAsBool(BlackboardKey.SelectedKeyName))
	{
		return;
	}

	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;

	if (USP_AttackComponent* Attack = ControlledPawn ? ControlledPawn->FindComponentByClass<USP_AttackComponent>() : nullptr)
	{
		// Does nothing while an attack runs or during the cooldown.
		Attack->StartAttack();
	}
}