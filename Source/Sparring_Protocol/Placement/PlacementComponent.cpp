#include "Placement/PlacementComponent.h"
#include "Placement/PlacementGhost.h"
#include "Placement/PlaceableDefinition.h"
#include "Arena/ArenaManager.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlacement, Log, All);

namespace
{
	constexpr uint64 HudKeyHeader = 7001;
	constexpr uint64 HudKeyStatus = 7002;
	constexpr uint64 HudKeyHelp = 7003;

	void ShowMessage(const FString& Message, const FColor& Color, float Duration = 3.f)
	{
		UE_LOG(LogPlacement, Log, TEXT("%s"), *Message);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
		}
	}
}

UPlacementComponent::UPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPlacementComponent::BeginPlay()
{
	Super::BeginPlay();

	Manager = ResolveManager();
	if (!Manager)
	{
		UE_LOG(LogPlacement, Warning, TEXT("No ArenaManager found for %s."), *GetNameSafe(GetOwner()));
		return;
	}

	Manager->OnBuildPhaseStarted.AddDynamic(this, &UPlacementComponent::HandleBuildPhaseStarted);
	Manager->OnBuildPhaseFinished.AddDynamic(this, &UPlacementComponent::HandleBuildPhaseFinished);
}

void UPlacementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Manager)
	{
		Manager->OnBuildPhaseStarted.RemoveDynamic(this, &UPlacementComponent::HandleBuildPhaseStarted);
		Manager->OnBuildPhaseFinished.RemoveDynamic(this, &UPlacementComponent::HandleBuildPhaseFinished);
	}
	ExitPlacementMode();

	Super::EndPlay(EndPlayReason);
}

void UPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bPlacementActive)
	{
		UpdatePreview();
	}
}

AArenaManager* UPlacementComponent::ResolveManager() const
{
	if (AArenaManager* OwnerManager = Cast<AArenaManager>(GetOwner()))
	{
		return OwnerManager;
	}

	TActorIterator<AArenaManager> It(GetWorld());
	return It ? *It : nullptr;
}

UPlaceableDefinition* UPlacementComponent::GetCurrentDefinition() const
{
	if (AvailableDefinitions.IsEmpty())
	{
		return nullptr;
	}
	return AvailableDefinitions[FMath::Clamp(CurrentDefinitionIndex, 0, AvailableDefinitions.Num() - 1)];
}

int32 UPlacementComponent::GetEffectiveTurns(const UPlaceableDefinition* Definition) const
{
	return (Definition && Definition->bAllowRotation) ? QuarterTurns : 0;
}

// ---- Build phase events -------------------------------------------------------

void UPlacementComponent::HandleBuildPhaseStarted(int32 PlayerSlots)
{
	if (PlayerSlots <= 0)
	{
		ShowMessage(TEXT("No building slots this round."), FColor::Yellow);
		return;
	}
	EnterPlacementMode();
}

void UPlacementComponent::HandleBuildPhaseFinished()
{
	ExitPlacementMode();
}

void UPlacementComponent::EnterPlacementMode()
{
	if (bPlacementActive)
	{
		return;
	}
	if (AvailableDefinitions.IsEmpty())
	{
		UE_LOG(LogPlacement, Warning, TEXT("AvailableDefinitions is empty: nothing to place."));
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	UWorld* World = GetWorld();
	if (!PC || !World)
	{
		return;
	}

	// Our own input component, pushed on top of the stack so it has priority over the pawn.
	PlacementInput = NewObject<UInputComponent>(PC, TEXT("PlacementInput"));
	PlacementInput->Priority = 100;
	PlacementInput->BindKey(ConfirmKey, IE_Pressed, this, &UPlacementComponent::ConfirmPlacement);
	PlacementInput->BindKey(ToggleGhostKey, IE_Pressed, this, &UPlacementComponent::ToggleGhost);
	PlacementInput->BindKey(RotateKey, IE_Pressed, this, &UPlacementComponent::RotateGhost);
	PlacementInput->BindKey(CycleKey, IE_Pressed, this, &UPlacementComponent::CycleDefinition);
	PlacementInput->BindKey(FinishKey, IE_Pressed, this, &UPlacementComponent::FinishBuilding);
	PC->PushInputComponent(PlacementInput);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Ghost = World->SpawnActor<APlacementGhost>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (Ghost)
	{
		Ghost->SetMaterials(GhostValidMaterial, GhostInvalidMaterial);
		Ghost->SetActorHiddenInGame(true);
	}

	bGhostEnabled = true;
	bHasAimCell = false;
	QuarterTurns = 0;
	LastReason = EPlacementFailReason::None;
	bPlacementActive = true;
	SetComponentTickEnabled(true);
}

void UPlacementComponent::ExitPlacementMode()
{
	if (!bPlacementActive)
	{
		return;
	}

	if (PlacementInput)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			PC->PopInputComponent(PlacementInput);
		}
		PlacementInput = nullptr;
	}

	if (Ghost)
	{
		Ghost->Destroy();
		Ghost = nullptr;
	}

	bPlacementActive = false;
	bHasAimCell = false;
	SetComponentTickEnabled(false);

	if (GEngine)
	{
		GEngine->RemoveOnScreenDebugMessage(HudKeyHeader);
		GEngine->RemoveOnScreenDebugMessage(HudKeyStatus);
		GEngine->RemoveOnScreenDebugMessage(HudKeyHelp);
	}
}

// ---- Player actions -----------------------------------------------------------

void UPlacementComponent::ConfirmPlacement()
{
	if (!bPlacementActive || !bGhostEnabled || !bHasAimCell || !Manager)
	{
		return;
	}

	UPlaceableDefinition* Definition = GetCurrentDefinition();
	if (!Definition)
	{
		return;
	}

	const FPlacementResult Result = Manager->TryPlace(Definition, LastAimCell, GetEffectiveTurns(Definition), EPlacementOwner::Player);
	if (!Result.bSuccess)
	{
		ShowMessage(GetFailReasonText(Result.FailReason).ToString(), FColor::Red, 2.f);
		return;
	}

	if (Manager->GetSlotsToPlace(EPlacementOwner::Player) <= 0)
	{
		ShowMessage(FString::Printf(TEXT("All slots used. Press [%s] to finish building."), *FinishKey.GetDisplayName().ToString()), FColor::Yellow);
	}
}

void UPlacementComponent::ToggleGhost()
{
	if (!bPlacementActive)
	{
		return;
	}
	bGhostEnabled = !bGhostEnabled;
	if (!bGhostEnabled && Ghost)
	{
		Ghost->SetActorHiddenInGame(true);
	}
}

void UPlacementComponent::RotateGhost()
{
	if (bPlacementActive)
	{
		QuarterTurns = (QuarterTurns + 1) % 4;
	}
}

void UPlacementComponent::CycleDefinition()
{
	if (bPlacementActive && !AvailableDefinitions.IsEmpty())
	{
		CurrentDefinitionIndex = (CurrentDefinitionIndex + 1) % AvailableDefinitions.Num();
		QuarterTurns = 0;
	}
}

void UPlacementComponent::FinishBuilding()
{
	if (bPlacementActive && Manager)
	{
		Manager->EndBuildPhase();
	}
}

// ---- Preview ------------------------------------------------------------------

void UPlacementComponent::UpdatePreview()
{
	UPlaceableDefinition* Definition = GetCurrentDefinition();
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	UWorld* World = GetWorld();
	if (!Manager || !Definition || !Ghost || !PC || !World)
	{
		return;
	}

	bHasAimCell = false;

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * MaxAimDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ArenaPlacementAim), false);
	Params.AddIgnoredActor(Ghost);
	if (APawn* Pawn = PC->GetPawn())
	{
		Params.AddIgnoredActor(Pawn);
	}

	const bool bHit = bGhostEnabled && World->LineTraceSingleByChannel(Hit, ViewLocation, TraceEnd, ECC_Visibility, Params);
	if (!bHit)
	{
		Ghost->SetActorHiddenInGame(true);
		UpdateHud(Definition, bGhostEnabled ? TEXT("Aim at the arena floor") : TEXT("Ghost off"), FColor::Yellow);
		return;
	}

	const FArenaGrid& Grid = Manager->GetGrid();
	const float CellSize = Grid.GetCellSize();
	const int32 Turns = GetEffectiveTurns(Definition);
	const FIntVector Footprint = Definition->GetRotatedFootprint(Turns);

	// Push the hit point out along the surface normal: on the floor this stays in the cell
	// hit, on the top of a block it gives the cell above, on a side face the neighbour cell.
	const FVector Probe = Hit.ImpactPoint + Hit.ImpactNormal * (CellSize * 0.5f);
	const FIntVector Aimed = Grid.WorldToCell(Probe);
	const FIntVector Cell(Aimed.X - (Footprint.X - 1) / 2, Aimed.Y - (Footprint.Y - 1) / 2, Aimed.Z);

	const EPlacementFailReason Reason = Manager->ValidatePlacement(Definition, Cell, Turns, EPlacementOwner::Player);
	const bool bValid = (Reason == EPlacementFailReason::None);

	LastAimCell = Cell;
	bHasAimCell = true;

	const FVector Center = Grid.CellToWorld(Cell) + FVector(
		(Footprint.X - 1) * CellSize * 0.5f,
		(Footprint.Y - 1) * CellSize * 0.5f,
		(Footprint.Z - 1) * CellSize * 0.5f);

	// The mesh uses the unrotated footprint and is rotated like the real element will be.
	const FVector UnrotatedBox(
		Definition->Footprint.X * CellSize,
		Definition->Footprint.Y * CellSize,
		Definition->Footprint.Z * CellSize);

	Ghost->SetActorHiddenInGame(false);
	Ghost->SetActorLocationAndRotation(Center, FRotator(0.f, 90.f * Turns, 0.f));
	Ghost->SetPreview(Definition->PreviewMesh, UnrotatedBox);
	Ghost->SetValid(bValid);

	if (bDrawDebugGhostBox)
	{
		const FVector HalfExtent(Footprint.X * CellSize * 0.5f, Footprint.Y * CellSize * 0.5f, Footprint.Z * CellSize * 0.5f);
		DrawDebugBox(World, Center, HalfExtent, bValid ? FColor::Green : FColor::Red, false, -1.f, 0, 4.f);
	}

	if (Reason != LastReason)
	{
		LastReason = Reason;
		OnPreviewChanged.Broadcast(Reason);
	}

	UpdateHud(Definition,
		bValid ? TEXT("Valid placement") : GetFailReasonText(Reason).ToString(),
		bValid ? FColor::Green : FColor::Red);
}

void UPlacementComponent::UpdateHud(const UPlaceableDefinition* Definition, const FString& Status, const FColor& StatusColor) const
{
	if (!bShowOnScreenHelp || !GEngine || !Manager)
	{
		return;
	}

	const FString Name = (Definition && !Definition->DisplayName.IsEmpty())
		? Definition->DisplayName.ToString()
		: GetNameSafe(Definition);

	const FString Lifetime = (Definition && Definition->LifetimeRounds > 0)
		? FString::Printf(TEXT("lasts %d rounds"), Definition->LifetimeRounds)
		: FString(TEXT("permanent"));

	GEngine->AddOnScreenDebugMessage(HudKeyHeader, 0.f, FColor::Cyan,
		FString::Printf(TEXT("BUILD MODE | %s | Slots left: %d / %d | %s | Rotation: %d deg"),
			*Name,
			Manager->GetSlotsToPlace(EPlacementOwner::Player),
			Manager->GetMaxAllowed(),
			*Lifetime,
			GetEffectiveTurns(Definition) * 90));

	GEngine->AddOnScreenDebugMessage(HudKeyStatus, 0.f, StatusColor, Status);

	GEngine->AddOnScreenDebugMessage(HudKeyHelp, 0.f, FColor::White,
		FString::Printf(TEXT("[%s] Place   [%s] Toggle ghost   [%s] Rotate   [%s] Next element   [%s] Finish building"),
			*ConfirmKey.GetDisplayName().ToString(),
			*ToggleGhostKey.GetDisplayName().ToString(),
			*RotateKey.GetDisplayName().ToString(),
			*CycleKey.GetDisplayName().ToString(),
			*FinishKey.GetDisplayName().ToString()));
}

FText UPlacementComponent::GetFailReasonText(EPlacementFailReason Reason)
{
	switch (Reason)
	{
	case EPlacementFailReason::None:               return FText::FromString(TEXT("Valid placement"));
	case EPlacementFailReason::NotInBuildPhase:    return FText::FromString(TEXT("Not in a build phase"));
	case EPlacementFailReason::InvalidDefinition:  return FText::FromString(TEXT("Invalid element"));
	case EPlacementFailReason::OutOfBounds:        return FText::FromString(TEXT("Outside the arena"));
	case EPlacementFailReason::CellOccupied:       return FText::FromString(TEXT("Space already occupied"));
	case EPlacementFailReason::TooCloseToSpawn:    return FText::FromString(TEXT("Too close to a spawn point"));
	case EPlacementFailReason::NoSupport:          return FText::FromString(TEXT("Needs support underneath"));
	case EPlacementFailReason::NoSlotsLeft:        return FText::FromString(TEXT("No slots left"));
	case EPlacementFailReason::BlockedByCharacter: return FText::FromString(TEXT("A character is in the way"));
	case EPlacementFailReason::BlocksPath:         return FText::FromString(TEXT("Blocks the only path between spawns"));
	case EPlacementFailReason::NoValidSlot:         return FText::FromString(TEXT("No valid place available"));
	default:                                       return FText::FromString(TEXT("Cannot place here"));
	}
}
