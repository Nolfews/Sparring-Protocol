#include "IA/BehaviorTree/BTService_EnemyMobility.h"
#include "IA/Components/SP_EnemyMobilityComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
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
	bNotifyCeaseRelevant = true; // without this, OnCeaseRelevant is never called

	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EnemyMobility, BlackboardKey), AActor::StaticClass());
}

void UBTService_EnemyMobility::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	const AActor* Goal = BlackboardComp ? Cast<AActor>(BlackboardComp->GetValueAsObject(BlackboardKey.SelectedKeyName)) : nullptr;
	USP_EnemyMobilityComponent* Mobility = FindMobility(OwnerComp);
	if (!Goal || !Mobility)
	{
		return;
	}

	// A player in the middle of a jump is not standing on a block.
	const ACharacter* GoalCharacter = Cast<ACharacter>(Goal);
	const bool bGoalIsGrounded = !GoalCharacter || !GoalCharacter->GetCharacterMovement()->IsFalling();

	Mobility->UpdateMobility(Goal->GetActorLocation(), bGoalIsGrounded);
}

void UBTService_EnemyMobility::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (USP_EnemyMobilityComponent* Mobility = FindMobility(OwnerComp))
	{
		Mobility->StopClimbing();
	}

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}