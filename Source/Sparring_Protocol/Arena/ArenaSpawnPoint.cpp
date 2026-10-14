#include "Arena/ArenaSpawnPoint.h"
#include "Components/ArrowComponent.h"

AArenaSpawnPoint::AArenaSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(Root);
	Arrow->ArrowSize = 3.f;
	Arrow->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
}

void AArenaSpawnPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Arrow)
	{
		Arrow->ArrowColor = (SpawnOwner == EPlacementOwner::Player) ? FColor::Green : FColor::Red;
		Arrow->MarkRenderStateDirty();
	}
}
