#include "Player/SP_PlayerCharacter.h"

#include "Abilities/SP_AbilityComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

ASP_PlayerCharacter::ASP_PlayerCharacter()
{
	// Tick is only used to smooth the camera height when sliding.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 96.f);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, StandingCameraHeight));
	FirstPersonCamera->bUsePawnControlRotation = true;

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->SetCastShadow(false);
	FirstPersonMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// The full body is for the other players / cameras, not for us.
	GetMesh()->SetOwnerNoSee(true);

	// FPS: the body turns with the mouse, not toward the movement.
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->MaxWalkSpeed = 900.f;
	Movement->BrakingDecelerationWalking = 2500.f;
}

void ASP_PlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		for (UInputMappingContext* Context : DefaultMappingContexts)
		{
			if (Context)
			{
				Subsystem->AddMappingContext(Context, 0);
			}
		}
	}
}

void ASP_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	if (MoveAction)
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASP_PlayerCharacter::Move);
	}
	if (LookAction)
	{
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASP_PlayerCharacter::Look);
	}
	if (MouseLookAction)
	{
		EnhancedInput->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ASP_PlayerCharacter::Look);
	}

	// Every ability component binds its own Input Action.
	TInlineComponentArray<USP_AbilityComponent*> Abilities(this);
	for (USP_AbilityComponent* Ability : Abilities)
	{
		Ability->BindInput(EnhancedInput);
	}
}

void ASP_PlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), Input.Y);
	AddMovementInput(GetActorRightVector(), Input.X);
}

void ASP_PlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void ASP_PlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// The capsule just went down: move the camera back up so it does not snap,
	// Tick then brings it smoothly to CrouchedCameraHeight.
	FVector Location = FirstPersonCamera->GetRelativeLocation();
	Location.Z += ScaledHalfHeightAdjust;
	FirstPersonCamera->SetRelativeLocation(Location);
}

void ASP_PlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	FVector Location = FirstPersonCamera->GetRelativeLocation();
	Location.Z -= ScaledHalfHeightAdjust;
	FirstPersonCamera->SetRelativeLocation(Location);
}

void ASP_PlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float TargetHeight = bIsCrouched ? CrouchedCameraHeight : StandingCameraHeight;
	FVector Location = FirstPersonCamera->GetRelativeLocation();
	if (!FMath::IsNearlyEqual(Location.Z, TargetHeight, 0.1f))
	{
		Location.Z = FMath::FInterpTo(Location.Z, TargetHeight, DeltaSeconds, CameraHeightInterpSpeed);
		FirstPersonCamera->SetRelativeLocation(Location);
	}
}

void ASP_PlayerCharacter::Revive()
{
	Super::Revive();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		EnableInput(PlayerController);
	}
}

void ASP_PlayerCharacter::HandleDeath(AActor* Killer)
{
	Super::HandleDeath(Killer);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		DisableInput(PlayerController);
	}
}
