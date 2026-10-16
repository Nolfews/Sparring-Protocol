#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlacementGhost.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

/**
 * Non-colliding preview of the element being placed.
 * Without materials assigned it stays invisible and the component's debug box is the only feedback.
 */
UCLASS()
class SPARRING_PROTOCOL_API APlacementGhost : public AActor
{
	GENERATED_BODY()

public:
	APlacementGhost();

	/** Fits the mesh (or a unit cube when null) to a box of the given world size in cm. */
	void SetPreview(UStaticMesh* PreviewMesh, const FVector& BoxSize);

	void SetMaterials(UMaterialInterface* InValidMaterial, UMaterialInterface* InInvalidMaterial);

	void SetValid(bool bInValid);

private:
	void ApplyVisuals();

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> FallbackMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ValidMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> InvalidMaterial;

	bool bValid = false;
};
