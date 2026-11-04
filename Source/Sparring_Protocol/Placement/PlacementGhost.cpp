#include "Placement/PlacementGhost.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

APlacementGhost::APlacementGhost()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCanEverAffectNavigation(false);
	Mesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		FallbackMesh = CubeMesh.Object;
		Mesh->SetStaticMesh(FallbackMesh);
	}
}

void APlacementGhost::SetPreview(UStaticMesh* PreviewMesh, const FVector& BoxSize)
{
	UStaticMesh* UsedMesh = PreviewMesh ? PreviewMesh : FallbackMesh.Get();
	if (!UsedMesh)
	{
		return;
	}

	Mesh->SetStaticMesh(UsedMesh);

	// Scale the mesh so its bounds fill the box, and cancel any off-center pivot.
	const FBoxSphereBounds Bounds = UsedMesh->GetBounds();
	const FVector MeshSize = Bounds.BoxExtent * 2.0;
	const FVector Scale(
		BoxSize.X / FMath::Max(MeshSize.X, 1.0),
		BoxSize.Y / FMath::Max(MeshSize.Y, 1.0),
		BoxSize.Z / FMath::Max(MeshSize.Z, 1.0));

	Mesh->SetRelativeScale3D(Scale);
	Mesh->SetRelativeLocation(-Bounds.Origin * Scale);
}

void APlacementGhost::SetMaterials(UMaterialInterface* InValidMaterial, UMaterialInterface* InInvalidMaterial)
{
	ValidMaterial = InValidMaterial;
	InvalidMaterial = InInvalidMaterial;
	ApplyVisuals();
}

void APlacementGhost::SetValid(bool bInValid)
{
	if (bValid != bInValid)
	{
		bValid = bInValid;
		ApplyVisuals();
	}
}

void APlacementGhost::ApplyVisuals()
{
	UMaterialInterface* Material = bValid ? ValidMaterial.Get() : InvalidMaterial.Get();
	if (Material)
	{
		Mesh->SetMaterial(0, Material);
		Mesh->SetVisibility(true);
	}
	else
	{
		Mesh->SetVisibility(false);
	}
}
