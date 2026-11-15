#pragma once

#include "CoreMinimal.h"
#include "Combat/SP_CombatCharacter.h"
#include "SP_PlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UMaterialInterface;
class USkeletalMeshComponent;
struct FInputActionValue;

/**
 * First person player.
 * Handles the camera, moving and looking around. Everything else comes from the ability
 * components added in the Blueprint: each one brings its own Input Action and is bound
 * automatically here, so adding an ability needs no code in this class.
 */
UCLASS()
class SPARRING_PROTOCOL_API ASP_PlayerCharacter : public ASP_CombatCharacter
{
	GENERATED_BODY()

public:
	ASP_PlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void Revive() override;

	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	/** Added to the player when he takes control (IMC_Default, IMC_MouseLook, IMC_Weapons...). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Gamepad look. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	/** Mouse look. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MouseLookAction;

	/** Camera height above the capsule center, standing and crouched (slide). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (Units = "cm"))
	float StandingCameraHeight = 64.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (Units = "cm"))
	float CrouchedCameraHeight = 40.f;

	/** Speed of the camera going down / up when sliding. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "1"))
	float CameraHeightInterpSpeed = 15.f;

	/**
	 * Post process material added to the camera (M_PP_Outline): draws the outline of everything
	 * written into the custom depth (the enemies). On the camera, so it works in every level.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UMaterialInterface> OutlineMaterial;

	/** Socket of the first person weapon where the projectiles leave. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponMuzzleSocketName = TEXT("Muzzle");

protected:
	virtual void BeginPlay() override;
	virtual void HandleDeath(AActor* Killer) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	/** Arms / weapon seen only by the player. Set the mesh in the Blueprint. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	/**
	 * The rifle seen in first person, attached to the camera. Set the mesh (SKM_Rifle) and place it
	 * in the Blueprint. Drawn with the First Person Field Of View / Scale of the camera, so it never
	 * goes through the walls. With a projectile weapon, the bullets leave from its muzzle.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> FirstPersonWeapon;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
};
