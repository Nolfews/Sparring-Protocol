#include "UI/GameHUDWidget.h"

#include "Arena/ArenaManager.h"
#include "Placement/PlacementComponent.h"
#include "Placement/PlaceableDefinition.h"
#include "Combat/SP_CombatCharacter.h"
#include "Combat/SP_HealthComponent.h"
#include "Abilities/SP_DashComponent.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"

namespace
{
	/** HitTestInvisible: the HUD never takes the mouse or the focus away from the game. */
	void SetShown(UWidget* Widget, bool bShown)
	{
		if (Widget)
		{
			Widget->SetVisibility(bShown ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	}

	void SetBarPercent(UProgressBar* Bar, float Value, float Max)
	{
		if (Bar)
		{
			Bar->SetPercent(Max > 0.f ? FMath::Clamp(Value / Max, 0.f, 1.f) : 0.f);
		}
	}
}

// ---- Lifetime ---------------------------------------------------------------------------------

void UGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		TActorIterator<AGameFlowManager> FlowIt(World);
		if (FlowIt)
		{
			GameFlow = *FlowIt;
		}

		TActorIterator<AArenaManager> ArenaIt(World);
		if (ArenaIt)
		{
			Arena = *ArenaIt;
		}

		// The placement component sits on the arena manager, or on any other actor.
		if (Arena)
		{
			Placement = Arena->FindComponentByClass<UPlacementComponent>();
		}
		if (!Placement)
		{
			for (TActorIterator<AActor> It(World); It && !Placement; ++It)
			{
				Placement = It->FindComponentByClass<UPlacementComponent>();
			}
		}
	}

	if (GameFlow)
	{
		GameFlow->OnStateChanged.AddDynamic(this, &UGameHUDWidget::HandleFlowStateChanged);
		GameFlow->OnRoundStarted.AddDynamic(this, &UGameHUDWidget::HandleRoundStarted);
		GameFlow->OnEnemySpawned.AddDynamic(this, &UGameHUDWidget::HandleEnemySpawned);
	}
	if (Arena)
	{
		Arena->OnBuildPhaseStarted.AddDynamic(this, &UGameHUDWidget::HandleBuildPhaseStarted);
		Arena->OnBuildPhaseFinished.AddDynamic(this, &UGameHUDWidget::HandleBuildPhaseFinished);
		Arena->OnElementPlaced.AddDynamic(this, &UGameHUDWidget::HandleElementPlaced);
		Arena->OnElementRemoved.AddDynamic(this, &UGameHUDWidget::HandleElementRemoved);
	}
	if (Placement)
	{
		Placement->OnSelectionChanged.AddDynamic(this, &UGameHUDWidget::HandleSelectionChanged);
		Placement->OnPreviewChanged.AddDynamic(this, &UGameHUDWidget::HandlePreviewChanged);
	}

	TryBindPlayer();
	if (GameFlow && GameFlow->GetEnemy())
	{
		BindEnemy(GameFlow->GetEnemy());
	}

	RefreshAll();
}

void UGameHUDWidget::NativeDestruct()
{
	if (GameFlow)
	{
		GameFlow->OnStateChanged.RemoveDynamic(this, &UGameHUDWidget::HandleFlowStateChanged);
		GameFlow->OnRoundStarted.RemoveDynamic(this, &UGameHUDWidget::HandleRoundStarted);
		GameFlow->OnEnemySpawned.RemoveDynamic(this, &UGameHUDWidget::HandleEnemySpawned);
	}
	if (Arena)
	{
		Arena->OnBuildPhaseStarted.RemoveDynamic(this, &UGameHUDWidget::HandleBuildPhaseStarted);
		Arena->OnBuildPhaseFinished.RemoveDynamic(this, &UGameHUDWidget::HandleBuildPhaseFinished);
		Arena->OnElementPlaced.RemoveDynamic(this, &UGameHUDWidget::HandleElementPlaced);
		Arena->OnElementRemoved.RemoveDynamic(this, &UGameHUDWidget::HandleElementRemoved);
	}
	if (Placement)
	{
		Placement->OnSelectionChanged.RemoveDynamic(this, &UGameHUDWidget::HandleSelectionChanged);
		Placement->OnPreviewChanged.RemoveDynamic(this, &UGameHUDWidget::HandlePreviewChanged);
	}
	if (PlayerHealth.IsValid())
	{
		PlayerHealth->OnHealthChanged.RemoveDynamic(this, &UGameHUDWidget::HandlePlayerHealthChanged);
	}
	if (EnemyHealth.IsValid())
	{
		EnemyHealth->OnHealthChanged.RemoveDynamic(this, &UGameHUDWidget::HandleEnemyHealthChanged);
	}

	Super::NativeDestruct();
}

void UGameHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// The player pawn may not be possessed yet when the HUD is created.
	if (!PlayerHealth.IsValid())
	{
		TryBindPlayer();
	}

	// Dashes: charges and recharge progress change over time, so they are read every frame.
	if (PlayerDash.IsValid())
	{
		const int32 Charges = PlayerDash->GetCharges();
		const int32 MaxCharges = PlayerDash->MaxCharges;
		const float Progress = PlayerDash->GetRechargeProgress();

		if (Charges != LastDashCharges || MaxCharges != LastDashMax || !FMath::IsNearlyEqual(Progress, LastDashProgress, 0.005f))
		{
			LastDashCharges = Charges;
			LastDashMax = MaxCharges;
			LastDashProgress = Progress;
			SetDashState(Charges, MaxCharges, Progress);
		}
	}
}

// ---- Binding to the characters --------------------------------------------------------------------

void UGameHUDWidget::TryBindPlayer()
{
	const ASP_CombatCharacter* Character = Cast<ASP_CombatCharacter>(GetOwningPlayerPawn());
	if (!Character)
	{
		return;
	}

	PlayerDash = Character->FindComponentByClass<USP_DashComponent>();

	if (USP_HealthComponent* Health = Character->GetHealthComponent())
	{
		PlayerHealth = Health;
		Health->OnHealthChanged.AddDynamic(this, &UGameHUDWidget::HandlePlayerHealthChanged);
		RefreshPlayerHealth();
	}
}

void UGameHUDWidget::BindEnemy(APawn* Enemy)
{
	if (EnemyHealth.IsValid())
	{
		EnemyHealth->OnHealthChanged.RemoveDynamic(this, &UGameHUDWidget::HandleEnemyHealthChanged);
		EnemyHealth.Reset();
	}

	if (const ASP_CombatCharacter* Character = Cast<ASP_CombatCharacter>(Enemy))
	{
		if (USP_HealthComponent* Health = Character->GetHealthComponent())
		{
			EnemyHealth = Health;
			Health->OnHealthChanged.AddDynamic(this, &UGameHUDWidget::HandleEnemyHealthChanged);
		}
	}
	RefreshEnemyHealth();
}

// ---- Event handlers ---------------------------------------------------------------------------------

void UGameHUDWidget::HandleFlowStateChanged(EGameFlowState NewState, EGameFlowState PreviousState)
{
	ApplyState(NewState);
	RefreshRound();
	RefreshSlots();
	RefreshSelection();
	RefreshKeyHints();
	RefreshTutorial(NewState);

	if (StatusText)
	{
		StatusText->SetText(FText::GetEmpty());
	}
}

void UGameHUDWidget::HandleRoundStarted(int32 RoundNumber)
{
	RefreshRound();
}

void UGameHUDWidget::HandleEnemySpawned(APawn* Enemy)
{
	BindEnemy(Enemy);
	ApplyState(GameFlow ? GameFlow->GetState() : EGameFlowState::Idle);
}

void UGameHUDWidget::HandleBuildPhaseStarted(int32 PlayerSlots)
{
	RefreshSlots();
	RefreshSelection();
	RefreshKeyHints();
}

void UGameHUDWidget::HandleBuildPhaseFinished()
{
	if (StatusText)
	{
		StatusText->SetText(FText::GetEmpty());
	}
}

void UGameHUDWidget::HandleElementPlaced(APlaceableElement* Element)
{
	RefreshSlots();
}

void UGameHUDWidget::HandleElementRemoved(APlaceableElement* Element)
{
	RefreshSlots();
}

void UGameHUDWidget::HandlePlayerHealthChanged(USP_HealthComponent* ChangedComponent, float NewHealth, float Delta)
{
	RefreshPlayerHealth();
}

void UGameHUDWidget::HandleEnemyHealthChanged(USP_HealthComponent* ChangedComponent, float NewHealth, float Delta)
{
	RefreshEnemyHealth();
}

void UGameHUDWidget::HandleSelectionChanged(UPlaceableDefinition* SelectedDefinition, int32 SelectedQuarterTurns)
{
	RefreshSelection();
}

void UGameHUDWidget::HandlePreviewChanged(EPlacementFailReason Reason)
{
	if (!StatusText)
	{
		return;
	}

	if (Reason == EPlacementFailReason::None)
	{
		StatusText->SetText(FText::GetEmpty());
	}
	else
	{
		StatusText->SetText(UPlacementComponent::GetFailReasonText(Reason));
		StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.25f, 0.25f)));
	}
}

// ---- Refresh ---------------------------------------------------------------------------------------

void UGameHUDWidget::RefreshAll()
{
	const EGameFlowState State = GameFlow ? GameFlow->GetState() : EGameFlowState::Idle;
	ApplyState(State);
	RefreshPlayerHealth();
	RefreshEnemyHealth();
	RefreshRound();
	RefreshSlots();
	RefreshSelection();
	RefreshKeyHints();
	RefreshTutorial(State);
}

void UGameHUDWidget::ApplyState(EGameFlowState State)
{
	const bool bCombat = (State == EGameFlowState::TutorialCombat || State == EGameFlowState::RoundCombat);
	const bool bBuild = (State == EGameFlowState::TutorialBuild || State == EGameFlowState::Build);
	const bool bTutorial = (State == EGameFlowState::TutorialCombat || State == EGameFlowState::TutorialBuild);

	SetShown(CombatPanel, bCombat);
	SetShown(EnemyPanel, bCombat && EnemyHealth.IsValid());
	SetShown(BuildPanel, bBuild);
	SetShown(TutorialPanel, bTutorial);
	SetShown(Crosshair, bCombat || bBuild);
}

void UGameHUDWidget::RefreshPlayerHealth()
{
	if (!PlayerHealth.IsValid())
	{
		return;
	}

	const float Current = PlayerHealth->GetHealth();
	const float Max = PlayerHealth->MaxHealth;
	SetBarPercent(PlayerHealthBar, Current, Max);

	if (PlayerHealthText)
	{
		PlayerHealthText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Current), FMath::CeilToInt(Max))));
	}
}

void UGameHUDWidget::RefreshEnemyHealth()
{
	if (!EnemyHealth.IsValid())
	{
		return;
	}

	const float Current = EnemyHealth->GetHealth();
	const float Max = EnemyHealth->MaxHealth;
	SetBarPercent(EnemyHealthBar, Current, Max);

	if (EnemyHealthText)
	{
		EnemyHealthText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Current), FMath::CeilToInt(Max))));
	}
}

void UGameHUDWidget::RefreshRound()
{
	if (!RoundText || !GameFlow)
	{
		return;
	}

	if (GameFlow->GetState() == EGameFlowState::TutorialCombat)
	{
		RoundText->SetText(TutorialRoundLabel);
	}
	else
	{
		RoundText->SetText(FText::Format(RoundFormat, FText::AsNumber(GameFlow->GetCurrentRound())));
	}
}

void UGameHUDWidget::RefreshSlots()
{
	if (!Arena)
	{
		return;
	}

	const int32 Used = Arena->GetUsedSlots(EPlacementOwner::Player);
	const int32 Max = Arena->GetMaxAllowed();

	if (SlotsText)
	{
		SlotsText->SetText(FText::Format(SlotsFormat, FText::AsNumber(Used), FText::AsNumber(Max)));
	}
	SetBarPercent(SlotsBar, static_cast<float>(Used), static_cast<float>(Max));
}

void UGameHUDWidget::RefreshSelection()
{
	if (!Placement)
	{
		return;
	}

	const UPlaceableDefinition* Definition = Placement->GetSelectedDefinition();
	const int32 Turns = Placement->GetSelectedQuarterTurns();

	if (ShapeNameText)
	{
		ShapeNameText->SetText(Definition ? Definition->DisplayName : FText::GetEmpty());
	}

	if (ShapeSizeText)
	{
		if (Definition)
		{
			const FIntVector Size = Definition->GetRotatedFootprint(Turns);
			ShapeSizeText->SetText(FText::FromString(Size.Z > 1
				? FString::Printf(TEXT("%d x %d x %d"), Size.X, Size.Y, Size.Z)
				: FString::Printf(TEXT("%d x %d"), Size.X, Size.Y)));
		}
		else
		{
			ShapeSizeText->SetText(FText::GetEmpty());
		}
	}

	if (ShapeIcon)
	{
		if (Definition && Definition->Icon)
		{
			ShapeIcon->SetBrushFromTexture(Definition->Icon);
			ShapeIcon->SetRenderTransformAngle(static_cast<float>(Turns) * 90.f);
			ShapeIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			ShapeIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UGameHUDWidget::RefreshKeyHints()
{
	if (!Placement)
	{
		return;
	}

	if (PlaceKeyText)
	{
		PlaceKeyText->SetText(GetKeyLabel(Placement->ConfirmKey));
	}
	if (RotateKeyText)
	{
		RotateKeyText->SetText(GetKeyLabel(Placement->RotateKey));
	}
	if (CycleKeyText)
	{
		CycleKeyText->SetText(GetKeyLabel(Placement->CycleKey));
	}
	if (FinishKeyText)
	{
		FinishKeyText->SetText(GetKeyLabel(Placement->FinishKey));
	}
}

void UGameHUDWidget::RefreshTutorial(EGameFlowState State)
{
	if (State == EGameFlowState::TutorialCombat)
	{
		if (TutorialTitleText) TutorialTitleText->SetText(CombatTutorialTitle);
		if (TutorialBodyText) TutorialBodyText->SetText(CombatTutorialBody);
	}
	else if (State == EGameFlowState::TutorialBuild)
	{
		if (TutorialTitleText) TutorialTitleText->SetText(BuildTutorialTitle);
		if (TutorialBodyText) TutorialBodyText->SetText(BuildTutorialBody);
	}
}

FText UGameHUDWidget::GetKeyLabel(const FKey& Key) const
{
	if (const FText* Override = KeyLabelOverrides.Find(Key))
	{
		return *Override;
	}
	if (Key == EKeys::LeftMouseButton)
	{
		return FText::FromString(TEXT("LMB"));
	}
	if (Key == EKeys::RightMouseButton)
	{
		return FText::FromString(TEXT("RMB"));
	}
	return FText::FromString(Key.GetDisplayName(false).ToString().ToUpper());
}

// ---- Dashes -------------------------------------------------------------------------------------------

void UGameHUDWidget::SetDashState(int32 AvailableCharges, int32 MaxCharges, float RechargeProgress)
{
	UProgressBar* Segments[3] = { DashSegment0.Get(), DashSegment1.Get(), DashSegment2.Get() };

	for (int32 Index = 0; Index < 3; ++Index)
	{
		UProgressBar* Segment = Segments[Index];
		if (!Segment)
		{
			continue;
		}

		Segment->SetVisibility(Index < MaxCharges ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

		float Fill = 0.f;
		if (Index < AvailableCharges)
		{
			Fill = 1.f;
		}
		else if (Index == AvailableCharges)
		{
			Fill = FMath::Clamp(RechargeProgress, 0.f, 1.f);
		}
		Segment->SetPercent(Fill);
	}
}
