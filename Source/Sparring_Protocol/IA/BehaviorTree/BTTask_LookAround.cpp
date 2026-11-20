#include "IA/BehaviorTree/BTTask_LookAround.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

namespace
{
	// How far in front of the enemy the focal point is placed. Only the direction matters.
	constexpr float FocalPointDistance = 1000.f;
}

UBTTask_LookAround::UBTTask_LookAround()
{
	NodeName = TEXT("Look Around");

	// TickTask turns the enemy, OnTaskFinished gives the rotation back (success or abort).
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	BlackboardKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_LookAround, BlackboardKey));
}

EBTNodeResult::Type UBTTask_LookAround::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	if (!BlackboardComp || !Pawn)
	{
		return EBTNodeResult::Failed;
	}

	// Not unlocked yet: the patrol goes on.
	if (!BlackboardComp->GetValueAsBool(BlackboardKey.SelectedKeyName))
	{
		return EBTNodeResult::Succeeded;
	}

	FBTLookAroundMemory* Memory = CastInstanceNodeMemory<FBTLookAroundMemory>(NodeMemory);
	Memory->StartYaw = Pawn->GetActorRotation().Yaw;
	Memory->TurnedAngle = 0.f;
	Memory->TurnDirection = FMath::RandBool() ? 1.f : -1.f;

	return EBTNodeResult::InProgress;
}

void UBTTask_LookAround::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FBTLookAroundMemory* Memory = CastInstanceNodeMemory<FBTLookAroundMemory>(NodeMemory);
	Memory->TurnedAngle = FMath::Min(Memory->TurnedAngle + TurnSpeed * DeltaSeconds, TurnAngle);

	// The focal point moves along a circle: the eyes look at it, the body follows at its Rotation Rate.
	const float Yaw = Memory->StartYaw + Memory->TurnDirection * Memory->TurnedAngle;
	const FVector LookDirection = FRotator(0.f, Yaw, 0.f).Vector();

	// Move priority, below Gameplay: a seen player (SetFocus in the controller) always wins.
	AIController->SetFocalPoint(Pawn->GetActorLocation() + LookDirection * FocalPointDistance, EAIFocusPriority::Move);

	if (Memory->TurnedAngle >= TurnAngle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_LookAround::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// Called on success and on abort (player seen): only our priority is cleared, never the chase focus.
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->ClearFocus(EAIFocusPriority::Move);
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

FString UBTTask_LookAround::GetStaticDescription() const
{
	return FString::Printf(TEXT("Turn %.0f° at %.0f°/s\nUnlocked by: %s"),
		TurnAngle, TurnSpeed, *BlackboardKey.SelectedKeyName.ToString());
}