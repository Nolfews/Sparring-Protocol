// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SP_Enemy_Character.generated.h"

class USP_MovementPaceComponent;
class USP_AttackComponent;

UCLASS()
class SPARRING_PROTOCOL_API ASP_Enemy_Character : public ACharacter
{
	GENERATED_BODY()


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USP_MovementPaceComponent> MovementPace;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USP_AttackComponent> Attack;

public:	
	ASP_Enemy_Character();

	// Called every frame

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
