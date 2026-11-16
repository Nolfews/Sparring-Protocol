#include "UI/SP_SettingsScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Score/SP_SettingsSubsystem.h"
#include "UI/SP_MenuButton.h"
#include "UI/SP_UIStyle.h"

namespace
{
	constexpr float SettingsControlWidth = 400.f;
	constexpr float SettingsRowHeight = 56.f;

	bool SettingsIsLeftKey(const FKey& Key)
	{
		return Key == EKeys::Left || Key == EKeys::Gamepad_DPad_Left || Key == EKeys::Gamepad_LeftStick_Left;
	}

	bool SettingsIsRightKey(const FKey& Key)
	{
		return Key == EKeys::Right || Key == EKeys::Gamepad_DPad_Right || Key == EKeys::Gamepad_LeftStick_Right;
	}

	FText SettingsSelectorText(const FString& Value)
	{
		return FText::FromString(FString::Printf(TEXT("<   %s   >"), *Value));
	}

	FString SettingsWindowModeName(EWindowMode::Type Mode)
	{
		switch (Mode)
		{
		case EWindowMode::Fullscreen: return TEXT("FULLSCREEN");
		case EWindowMode::WindowedFullscreen: return TEXT("BORDERLESS");
		default: return TEXT("WINDOWED");
		}
	}
}

USP_SettingsScreen::USP_SettingsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MenuButtonClass = TSoftClassPtr<USP_MenuButton>(FSoftObjectPath(TEXT("/Game/UI/Widgets/WBP_MenuButton.WBP_MenuButton_C")));
}

USP_SettingsSubsystem* USP_SettingsScreen::GetSettings() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<USP_SettingsSubsystem>() : nullptr;
}

// ---- Building the screen ------------------------------------------------------

TSharedRef<SWidget> USP_SettingsScreen::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildTree();
	}
	return Super::RebuildWidget();
}

USP_MenuButton* USP_SettingsScreen::MakeButton(const FText& Label)
{
	UClass* ButtonClass = MenuButtonClass.LoadSynchronous();
	if (!ButtonClass)
	{
		return nullptr;
	}
	USP_MenuButton* Button = WidgetTree->ConstructWidget<USP_MenuButton>(ButtonClass);
	Button->ButtonLabel = Label;
	return Button;
}

void USP_SettingsScreen::AddSection(UVerticalBox* Column, const FText& Title)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Text->SetText(Title);
	Text->SetFont(SPUIStyle::BodyFont(18, TEXT("Bold"), 300));
	Text->SetColorAndOpacity(FSlateColor(SPUIStyle::Violet()));

	UVerticalBoxSlot* TextSlot = Column->AddChildToVerticalBox(Text);
	TextSlot->SetPadding(FMargin(0.f, 18.f, 0.f, 6.f));
}

USlider* USP_SettingsScreen::AddSliderRow(UVerticalBox* Column, const FText& Label, float Min, float Max, float Step, TObjectPtr<UTextBlock>& OutValueText)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

	UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	LabelText->SetText(Label);
	LabelText->SetFont(SPUIStyle::BodyFont(24, TEXT("SemiBold"), 120));
	LabelText->SetColorAndOpacity(FSlateColor(SPUIStyle::TextLight()));
	UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(LabelText);
	LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	LabelSlot->SetVerticalAlignment(VAlign_Center);

	USizeBox* ControlBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ControlBox->SetWidthOverride(SettingsControlWidth);
	ControlBox->SetHeightOverride(SettingsRowHeight);
	Row->AddChildToHorizontalBox(ControlBox);

	UHorizontalBox* Control = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	ControlBox->AddChild(Control);

	USlider* Slider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass());
	Slider->SetMinValue(Min);
	Slider->SetMaxValue(Max);
	Slider->SetStepSize(Step);
	Slider->SetSliderBarColor(SPUIStyle::Outline());
	Slider->SetSliderHandleColor(SPUIStyle::Green());
	UHorizontalBoxSlot* SliderSlot = Control->AddChildToHorizontalBox(Slider);
	SliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	SliderSlot->SetVerticalAlignment(VAlign_Center);

	USizeBox* ValueBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ValueBox->SetWidthOverride(72.f);
	UTextBlock* ValueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	ValueText->SetFont(SPUIStyle::BodyFont(24, TEXT("Bold")));
	ValueText->SetColorAndOpacity(FSlateColor(SPUIStyle::Gold()));
	ValueText->SetJustification(ETextJustify::Right);
	ValueBox->AddChild(ValueText);
	UHorizontalBoxSlot* ValueSlot = Control->AddChildToHorizontalBox(ValueBox);
	ValueSlot->SetVerticalAlignment(VAlign_Center);
	OutValueText = ValueText;

	UVerticalBoxSlot* RowSlot = Column->AddChildToVerticalBox(Row);
	RowSlot->SetPadding(FMargin(0.f, 3.f));

	Rows.Add({ LabelText, Slider });
	return Slider;
}

USP_MenuButton* USP_SettingsScreen::AddSelectorRow(UVerticalBox* Column, const FText& Label)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

	UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	LabelText->SetText(Label);
	LabelText->SetFont(SPUIStyle::BodyFont(24, TEXT("SemiBold"), 120));
	LabelText->SetColorAndOpacity(FSlateColor(SPUIStyle::TextLight()));
	UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(LabelText);
	LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	LabelSlot->SetVerticalAlignment(VAlign_Center);

	USP_MenuButton* Button = MakeButton(FText::GetEmpty());
	if (Button)
	{
		USizeBox* ControlBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		ControlBox->SetWidthOverride(SettingsControlWidth);
		ControlBox->AddChild(Button);
		Row->AddChildToHorizontalBox(ControlBox);
		Rows.Add({ LabelText, Button->GetFocusTarget() });
	}

	UVerticalBoxSlot* RowSlot = Column->AddChildToVerticalBox(Row);
	RowSlot->SetPadding(FMargin(0.f, 3.f));
	return Button;
}

void USP_SettingsScreen::BuildTree()
{
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	WidgetTree->RootWidget = Root;

	UImage* Background = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Background->SetColorAndOpacity(SPUIStyle::Background().CopyWithNewOpacity(0.96f));
	UOverlaySlot* BackgroundSlot = Root->AddChildToOverlay(Background);
	BackgroundSlot->SetHorizontalAlignment(HAlign_Fill);
	BackgroundSlot->SetVerticalAlignment(VAlign_Fill);

	USizeBox* ColumnBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ColumnBox->SetWidthOverride(900.f);
	UOverlaySlot* ColumnSlot = Root->AddChildToOverlay(ColumnBox);
	ColumnSlot->SetHorizontalAlignment(HAlign_Center);
	ColumnSlot->SetVerticalAlignment(VAlign_Center);

	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	ColumnBox->AddChild(Column);

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Title->SetText(NSLOCTEXT("SPSettings", "Title", "SETTINGS"));
	Title->SetFont(SPUIStyle::TitleFont(64));
	Title->SetColorAndOpacity(FSlateColor(SPUIStyle::Violet()));
	UVerticalBoxSlot* TitleSlot = Column->AddChildToVerticalBox(Title);
	TitleSlot->SetHorizontalAlignment(HAlign_Center);
	TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

	AddSection(Column, NSLOCTEXT("SPSettings", "Controls", "CONTROLS"));
	SensitivitySlider = AddSliderRow(Column, NSLOCTEXT("SPSettings", "Sensitivity", "MOUSE SENSITIVITY"),
		USP_SettingsSubsystem::MinSensitivity, USP_SettingsSubsystem::MaxSensitivity, 0.05f, SensitivityValue);
	InvertButton = AddSelectorRow(Column, NSLOCTEXT("SPSettings", "InvertY", "INVERT Y AXIS"));

	AddSection(Column, NSLOCTEXT("SPSettings", "Audio", "AUDIO"));
	MasterSlider = AddSliderRow(Column, NSLOCTEXT("SPSettings", "Master", "MASTER VOLUME"), 0.f, 100.f, 5.f, MasterValue);
	MusicSlider = AddSliderRow(Column, NSLOCTEXT("SPSettings", "Music", "MUSIC"), 0.f, 100.f, 5.f, MusicValue);
	EffectsSlider = AddSliderRow(Column, NSLOCTEXT("SPSettings", "Effects", "EFFECTS"), 0.f, 100.f, 5.f, EffectsValue);

	AddSection(Column, NSLOCTEXT("SPSettings", "Video", "VIDEO"));
	ResolutionButton = AddSelectorRow(Column, NSLOCTEXT("SPSettings", "Resolution", "RESOLUTION"));
	WindowModeButton = AddSelectorRow(Column, NSLOCTEXT("SPSettings", "DisplayMode", "DISPLAY MODE"));
	VSyncButton = AddSelectorRow(Column, NSLOCTEXT("SPSettings", "VSync", "VSYNC"));

	// APPLY VIDEO   BACK
	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	ApplyButton = MakeButton(NSLOCTEXT("SPSettings", "Apply", "APPLY VIDEO"));
	BackButton = MakeButton(NSLOCTEXT("SPSettings", "Back", "BACK"));
	if (ApplyButton)
	{
		UHorizontalBoxSlot* ApplySlot = Buttons->AddChildToHorizontalBox(ApplyButton);
		ApplySlot->SetPadding(FMargin(0.f, 0.f, 16.f, 0.f));
	}
	if (BackButton)
	{
		Buttons->AddChildToHorizontalBox(BackButton);
	}
	UVerticalBoxSlot* ButtonsSlot = Column->AddChildToVerticalBox(Buttons);
	ButtonsSlot->SetHorizontalAlignment(HAlign_Center);
	ButtonsSlot->SetPadding(FMargin(0.f, 28.f, 0.f, 12.f));

	UTextBlock* Hint = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Hint->SetText(NSLOCTEXT("SPSettings", "Hint", "UP / DOWN  NAVIGATE      LEFT / RIGHT  CHANGE      ESC  BACK"));
	Hint->SetFont(SPUIStyle::BodyFont(16, TEXT("SemiBold"), 160));
	Hint->SetColorAndOpacity(FSlateColor(SPUIStyle::TextMuted()));
	UVerticalBoxSlot* HintSlot = Column->AddChildToVerticalBox(Hint);
	HintSlot->SetHorizontalAlignment(HAlign_Center);
}

// ---- Values -------------------------------------------------------------------

void USP_SettingsScreen::NativeConstruct()
{
	Super::NativeConstruct();

	LoadValues();
	RefreshTexts();

	// Bound after the values are set, so loading does not count as a change.
	if (SensitivitySlider) { SensitivitySlider->OnValueChanged.AddUniqueDynamic(this, &USP_SettingsScreen::HandleSensitivityChanged); }
	if (MasterSlider) { MasterSlider->OnValueChanged.AddUniqueDynamic(this, &USP_SettingsScreen::HandleMasterChanged); }
	if (MusicSlider) { MusicSlider->OnValueChanged.AddUniqueDynamic(this, &USP_SettingsScreen::HandleMusicChanged); }
	if (EffectsSlider) { EffectsSlider->OnValueChanged.AddUniqueDynamic(this, &USP_SettingsScreen::HandleEffectsChanged); }
	if (InvertButton) { InvertButton->OnPressed.AddUniqueDynamic(this, &USP_SettingsScreen::HandleInvertPressed); }
	if (ResolutionButton) { ResolutionButton->OnPressed.AddUniqueDynamic(this, &USP_SettingsScreen::HandleResolutionPressed); }
	if (WindowModeButton) { WindowModeButton->OnPressed.AddUniqueDynamic(this, &USP_SettingsScreen::HandleWindowModePressed); }
	if (VSyncButton) { VSyncButton->OnPressed.AddUniqueDynamic(this, &USP_SettingsScreen::HandleVSyncPressed); }
	if (ApplyButton) { ApplyButton->OnPressed.AddUniqueDynamic(this, &USP_SettingsScreen::HandleApplyPressed); }
	if (BackButton) { BackButton->OnPressed.AddUniqueDynamic(this, &USP_SettingsScreen::HandleBackPressed); }
}

void USP_SettingsScreen::LoadValues()
{
	if (const USP_SettingsSubsystem* Settings = GetSettings())
	{
		if (SensitivitySlider) { SensitivitySlider->SetValue(Settings->GetMouseSensitivity()); }
		if (MasterSlider) { MasterSlider->SetValue(Settings->GetVolume(ESPVolumeChannel::Master)); }
		if (MusicSlider) { MusicSlider->SetValue(Settings->GetVolume(ESPVolumeChannel::Music)); }
		if (EffectsSlider) { EffectsSlider->SetValue(Settings->GetVolume(ESPVolumeChannel::Effects)); }
	}

	UGameUserSettings* VideoSettings = UGameUserSettings::GetGameUserSettings();
	const FIntPoint CurrentResolution = VideoSettings ? VideoSettings->GetScreenResolution() : FIntPoint(1920, 1080);
	WindowMode = VideoSettings ? VideoSettings->GetFullscreenMode() : EWindowMode::Fullscreen;
	bVSync = VideoSettings && VideoSettings->IsVSyncEnabled();

	Resolutions.Reset();
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	Resolutions.AddUnique(CurrentResolution);
	Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B)
	{
		return A.X != B.X ? A.X < B.X : A.Y < B.Y;
	});
	ResolutionIndex = FMath::Max(0, Resolutions.IndexOfByKey(CurrentResolution));
}

void USP_SettingsScreen::RefreshTexts()
{
	const USP_SettingsSubsystem* Settings = GetSettings();

	if (SensitivityValue && SensitivitySlider)
	{
		SensitivityValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), SensitivitySlider->GetValue())));
	}
	if (MasterValue && MasterSlider) { MasterValue->SetText(FText::AsNumber(FMath::RoundToInt(MasterSlider->GetValue()))); }
	if (MusicValue && MusicSlider) { MusicValue->SetText(FText::AsNumber(FMath::RoundToInt(MusicSlider->GetValue()))); }
	if (EffectsValue && EffectsSlider) { EffectsValue->SetText(FText::AsNumber(FMath::RoundToInt(EffectsSlider->GetValue()))); }

	if (InvertButton)
	{
		InvertButton->SetLabel(SettingsSelectorText((Settings && Settings->GetInvertY()) ? TEXT("ON") : TEXT("OFF")));
	}
	if (ResolutionButton && Resolutions.IsValidIndex(ResolutionIndex))
	{
		const FIntPoint& Resolution = Resolutions[ResolutionIndex];
		ResolutionButton->SetLabel(SettingsSelectorText(FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y)));
	}
	if (WindowModeButton)
	{
		WindowModeButton->SetLabel(SettingsSelectorText(SettingsWindowModeName(WindowMode)));
	}
	if (VSyncButton)
	{
		VSyncButton->SetLabel(SettingsSelectorText(bVSync ? TEXT("ON") : TEXT("OFF")));
	}
}

// ---- Input ----------------------------------------------------------------------

void USP_SettingsScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	for (const FRow& Row : Rows)
	{
		if (UTextBlock* Label = Row.Label.Get())
		{
			const UWidget* Target = Row.FocusTarget.Get();
			const bool bFocused = Target && Target->HasKeyboardFocus();
			Label->SetColorAndOpacity(FSlateColor(bFocused ? SPUIStyle::Green() : SPUIStyle::TextLight()));
		}
	}
}

FReply USP_SettingsScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Left / right change the focused slider or selector.
	const FKey Key = InKeyEvent.GetKey();
	const int32 Direction = SettingsIsLeftKey(Key) ? -1 : (SettingsIsRightKey(Key) ? +1 : 0);
	if (Direction != 0 && (StepFocusedSlider(Direction) || CycleFocusedSelector(Direction)))
	{
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

bool USP_SettingsScreen::StepFocusedSlider(int32 Direction)
{
	USlider* Sliders[] = { SensitivitySlider, MasterSlider, MusicSlider, EffectsSlider };
	for (USlider* Slider : Sliders)
	{
		if (!Slider || !Slider->HasKeyboardFocus())
		{
			continue;
		}

		const bool bIsSensitivity = (Slider == SensitivitySlider);
		const float Step = bIsSensitivity ? 0.05f : 5.f;
		const float Min = bIsSensitivity ? USP_SettingsSubsystem::MinSensitivity : 0.f;
		const float Max = bIsSensitivity ? USP_SettingsSubsystem::MaxSensitivity : 100.f;
		const float NewValue = FMath::Clamp(Slider->GetValue() + Direction * Step, Min, Max);
		Slider->SetValue(NewValue);

		// SetValue does not fire OnValueChanged: apply the change by hand.
		if (Slider == SensitivitySlider) { HandleSensitivityChanged(NewValue); }
		else if (Slider == MasterSlider) { HandleMasterChanged(NewValue); }
		else if (Slider == MusicSlider) { HandleMusicChanged(NewValue); }
		else { HandleEffectsChanged(NewValue); }
		return true;
	}
	return false;
}

bool USP_SettingsScreen::CycleFocusedSelector(int32 Direction)
{
	auto HasFocus = [](const USP_MenuButton* Button)
	{
		const UWidget* Target = Button ? Button->GetFocusTarget() : nullptr;
		return Target && Target->HasKeyboardFocus();
	};

	if (HasFocus(InvertButton))
	{
		HandleInvertPressed();
		return true;
	}
	if (HasFocus(ResolutionButton))
	{
		CycleResolution(Direction);
		return true;
	}
	if (HasFocus(WindowModeButton))
	{
		CycleWindowMode(Direction);
		return true;
	}
	if (HasFocus(VSyncButton))
	{
		HandleVSyncPressed();
		return true;
	}
	return false;
}

UWidget* USP_SettingsScreen::GetDefaultFocusWidget() const
{
	return SensitivitySlider ? static_cast<UWidget*>(SensitivitySlider.Get()) : static_cast<UWidget*>(BackButton.Get());
}

bool USP_SettingsScreen::HandleBack()
{
	CloseScreen();
	return true;
}

void USP_SettingsScreen::CloseScreen()
{
	if (USP_SettingsSubsystem* Settings = GetSettings())
	{
		Settings->SaveSettings();
	}
	Super::CloseScreen();
}

// ---- Changes ----------------------------------------------------------------------

void USP_SettingsScreen::HandleSensitivityChanged(float Value)
{
	if (USP_SettingsSubsystem* Settings = GetSettings())
	{
		Settings->SetMouseSensitivity(Value);
	}
	RefreshTexts();
}

void USP_SettingsScreen::HandleMasterChanged(float Value)
{
	if (USP_SettingsSubsystem* Settings = GetSettings())
	{
		Settings->SetVolume(ESPVolumeChannel::Master, Value);
	}
	RefreshTexts();
}

void USP_SettingsScreen::HandleMusicChanged(float Value)
{
	if (USP_SettingsSubsystem* Settings = GetSettings())
	{
		Settings->SetVolume(ESPVolumeChannel::Music, Value);
	}
	RefreshTexts();
}

void USP_SettingsScreen::HandleEffectsChanged(float Value)
{
	if (USP_SettingsSubsystem* Settings = GetSettings())
	{
		Settings->SetVolume(ESPVolumeChannel::Effects, Value);
	}
	RefreshTexts();
}

void USP_SettingsScreen::HandleInvertPressed()
{
	if (USP_SettingsSubsystem* Settings = GetSettings())
	{
		Settings->SetInvertY(!Settings->GetInvertY());
	}
	RefreshTexts();
}

void USP_SettingsScreen::CycleResolution(int32 Direction)
{
	if (Resolutions.Num() > 0)
	{
		ResolutionIndex = (ResolutionIndex + Direction + Resolutions.Num()) % Resolutions.Num();
	}
	RefreshTexts();
}

void USP_SettingsScreen::CycleWindowMode(int32 Direction)
{
	static const EWindowMode::Type Modes[] = { EWindowMode::Fullscreen, EWindowMode::WindowedFullscreen, EWindowMode::Windowed };
	constexpr int32 ModeCount = 3;
	int32 Current = 0;
	for (int32 Index = 0; Index < ModeCount; ++Index)
	{
		if (Modes[Index] == WindowMode)
		{
			Current = Index;
		}
	}
	WindowMode = Modes[(Current + Direction + ModeCount) % ModeCount];
	RefreshTexts();
}

void USP_SettingsScreen::HandleResolutionPressed()
{
	CycleResolution(+1);
}

void USP_SettingsScreen::HandleWindowModePressed()
{
	CycleWindowMode(+1);
}

void USP_SettingsScreen::HandleVSyncPressed()
{
	bVSync = !bVSync;
	RefreshTexts();
}

void USP_SettingsScreen::HandleApplyPressed()
{
	UGameUserSettings* VideoSettings = UGameUserSettings::GetGameUserSettings();
	if (!VideoSettings)
	{
		return;
	}

	if (Resolutions.IsValidIndex(ResolutionIndex))
	{
		VideoSettings->SetScreenResolution(Resolutions[ResolutionIndex]);
	}
	VideoSettings->SetFullscreenMode(WindowMode);
	VideoSettings->SetVSyncEnabled(bVSync);
	VideoSettings->ApplySettings(false);
}

void USP_SettingsScreen::HandleBackPressed()
{
	CloseScreen();
}
