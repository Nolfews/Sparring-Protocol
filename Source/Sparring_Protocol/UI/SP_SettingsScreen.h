#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericWindow.h"
#include "UI/SP_MenuScreen.h"
#include "SP_SettingsScreen.generated.h"

class UPanelWidget;
class USlider;
class UTextBlock;
class UVerticalBox;
class USP_MenuButton;
class USP_SettingsSubsystem;

/**
 * Settings, opened from the main menu and the pause menu. Built entirely in code.
 *  - controls: mouse sensitivity, invert Y
 *  - audio: master, music, effects (Wwise), applied while moving the slider
 *  - video: resolution, display mode, VSync, applied with APPLY VIDEO
 * Arrows / D-pad up-down: navigate, left-right: change the value. Escape / B: back (saves).
 */
UCLASS()
class SPARRING_PROTOCOL_API USP_SettingsScreen : public USP_MenuScreen
{
	GENERATED_BODY()

public:
	USP_SettingsScreen(const FObjectInitializer& ObjectInitializer);

	/** Look of the buttons (WBP_MenuButton by default). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TSoftClassPtr<USP_MenuButton> MenuButtonClass;

	virtual void CloseScreen() override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual UWidget* GetDefaultFocusWidget() const override;
	virtual bool HandleBack() override;

private:
	/** A label on the left, a control on the right; the label turns green when the control has the focus. */
	struct FRow
	{
		TWeakObjectPtr<UTextBlock> Label;
		TWeakObjectPtr<UWidget> FocusTarget;
	};

	void BuildTree();
	void AddSection(UVerticalBox* Column, const FText& Title);
	USlider* AddSliderRow(UVerticalBox* Column, const FText& Label, float Min, float Max, float Step, TObjectPtr<UTextBlock>& OutValueText);
	USP_MenuButton* AddSelectorRow(UVerticalBox* Column, const FText& Label);
	USP_MenuButton* MakeButton(const FText& Label);

	void LoadValues();
	void RefreshTexts();
	bool StepFocusedSlider(int32 Direction);
	bool CycleFocusedSelector(int32 Direction);
	void CycleResolution(int32 Direction);
	void CycleWindowMode(int32 Direction);
	USP_SettingsSubsystem* GetSettings() const;

	UFUNCTION() void HandleSensitivityChanged(float Value);
	UFUNCTION() void HandleMasterChanged(float Value);
	UFUNCTION() void HandleMusicChanged(float Value);
	UFUNCTION() void HandleEffectsChanged(float Value);
	UFUNCTION() void HandleInvertPressed();
	UFUNCTION() void HandleResolutionPressed();
	UFUNCTION() void HandleWindowModePressed();
	UFUNCTION() void HandleVSyncPressed();
	UFUNCTION() void HandleApplyPressed();
	UFUNCTION() void HandleBackPressed();

	UPROPERTY(Transient) TObjectPtr<USlider> SensitivitySlider;
	UPROPERTY(Transient) TObjectPtr<USlider> MasterSlider;
	UPROPERTY(Transient) TObjectPtr<USlider> MusicSlider;
	UPROPERTY(Transient) TObjectPtr<USlider> EffectsSlider;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SensitivityValue;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> MasterValue;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> MusicValue;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> EffectsValue;
	UPROPERTY(Transient) TObjectPtr<USP_MenuButton> InvertButton;
	UPROPERTY(Transient) TObjectPtr<USP_MenuButton> ResolutionButton;
	UPROPERTY(Transient) TObjectPtr<USP_MenuButton> WindowModeButton;
	UPROPERTY(Transient) TObjectPtr<USP_MenuButton> VSyncButton;
	UPROPERTY(Transient) TObjectPtr<USP_MenuButton> ApplyButton;
	UPROPERTY(Transient) TObjectPtr<USP_MenuButton> BackButton;

	TArray<FRow> Rows;

	// Video settings waiting for APPLY VIDEO.
	TArray<FIntPoint> Resolutions;
	int32 ResolutionIndex = 0;
	EWindowMode::Type WindowMode = EWindowMode::Fullscreen;
	bool bVSync = false;
};
