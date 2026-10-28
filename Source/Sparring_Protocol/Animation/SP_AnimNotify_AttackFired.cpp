#include "Animation/SP_AnimNotify_AttackFired.h"
#include "IA/Components/SP_AttackComponent.h"
#include "Components/SkeletalMeshComponent.h"

void USP_AnimNotify_AttackFired::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// Also fires in the editor preview, where the owner has no attack component.
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (USP_AttackComponent* Attack = Owner ? Owner->FindComponentByClass<USP_AttackComponent>() : nullptr)
	{
		Attack->NotifyAttackFired();
	}
}
