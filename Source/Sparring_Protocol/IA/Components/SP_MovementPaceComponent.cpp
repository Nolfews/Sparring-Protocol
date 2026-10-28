#include "IA/Components/SP_MovementPaceComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void USP_MovementPaceComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedMovement = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	SetPace(ESP_MovementPace::Walk);
}

void USP_MovementPaceComponent::SetPace(ESP_MovementPace NewPace)
{
	CurrentPace = NewPace;

	if (CachedMovement)
	{
		switch (CurrentPace)
		{
		case ESP_MovementPace::Walk:
			CachedMovement->MaxWalkSpeed = WalkSpeed;
			break;

		case ESP_MovementPace::Run:
			CachedMovement->MaxWalkSpeed = RunSpeed;
			break;

		default:
			break;
		}
	}
}
