#include "IA/BehaviorTree/BTService_EnemyMobility.h"
#include "IA/Components/SP_EnemyMobilityComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
	USP_EnemyMobilityComponent* FindMobility(const UBehaviorTreeComponent& OwnerComp)
	{
		const AAIController* AIController = OwnerComp.GetAIOwner();
		const APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
		return Pawn ? Pawn->FindComponentByClass<USP_EnemyMobilityComponent>() : nullptr;
	}
}

UBTService_EnemyMobility::UBTService_EnemyMobility()
{
	NodeName = TEXT("Enemy Mobility");
	Interval = 0.15f;
	RandomDeviation = 0.05f;

	// Both kinds of key appear in the dropdown: Chase uses TargetActor, Investigate LastKnowLocation.
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EnemyMobility, BlackboardKey), AActor::StaticClass());
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EnemyMobility, BlackboardKey));
}

void UBTService_EnemyMobility::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	USP_EnemyMobilityComponent* Mobility = FindMobility(OwnerComp);
	if (!BlackboardComp || !Mobility)
	{
		return;
	}

	FVector GoalLocation;
	bool bGoalIsGrounded = true;

	if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		// A position: the controller only writes it while the player stands on something.
		GoalLocation = BlackboardComp->GetValueAsVector(BlackboardKey.SelectedKeyName);
		if (!FAISystem::IsValidLocation(GoalLocation))
		{
			return;
		}
	}
	else
	{
		const AActor* Goal = Cast<AActor>(BlackboardComp->GetValueAsObject(BlackboardKey.SelectedKeyName));
		if (!Goal)
		{
			return;
		}

		GoalLocation = Goal->GetActorLocation();

		// A player in the middle of a jump is not standing on a block.
		const ACharacter* GoalCharacter = Cast<ACharacter>(Goal);
		bGoalIsGrounded = !GoalCharacter || !GoalCharacter->GetCharacterMovement()->IsFalling();
	}

	Mobility->UpdateMobility(GoalLocation, bGoalIsGrounded);
}