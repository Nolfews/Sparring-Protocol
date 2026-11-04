#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SP_MovementPaceComponent.generated.h"

class UCharacterMovementComponent;

UENUM(BlueprintType)
enum class ESP_MovementPace : uint8
{
	Walk,
	Run
};

/** Holds the walk / run speeds of a character and applies them to its CharacterMovement. */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class SPARRING_PROTOCOL_API USP_MovementPaceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetPace(ESP_MovementPace NewPace);

	UFUNCTION(BlueprintPure, Category = "Movement")
	ESP_MovementPace GetPace() const { return CurrentPace; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0"))
	float WalkSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0"))
	float RunSpeed = 500.f;

private:
	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> CachedMovement;

	ESP_MovementPace CurrentPace = ESP_MovementPace::Walk;
};
