#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SP_AnimNotify_AttackFired.generated.h"

/** Place it on the frame where the attack lands: forwards to the owner's USP_AttackComponent. */
UCLASS(meta = (DisplayName = "Attack Fired"))
class SPARRING_PROTOCOL_API USP_AnimNotify_AttackFired : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
