#include "UI/SP_MenuButton.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

void USP_MenuButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// The tile itself can receive the focus and passes it to the inner button.
	SetIsFocusable(true);
}

void USP_MenuButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (LabelText)
	{
		LabelText->SetText(ButtonLabel);
		LabelText->SetColorAndOpacity(TextColor);
	}
}

void USP_MenuButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (!MainButton)
	{
		return;
	}

	MainButton->OnClicked.AddUniqueDynamic(this, &USP_MenuButton::HandleClicked);
	MainButton->OnHovered.AddUniqueDynamic(this, &USP_MenuButton::HandleHovered);

	// The "lit" style is the Hovered look of the Blueprint, used as the normal look.
	DefaultStyle = MainButton->GetStyle();
	HighlightStyle = DefaultStyle;
	HighlightStyle.Normal = DefaultStyle.Hovered;
	bStylesReady = true;

	bHighlighted = false;
	ApplyHighlight(false);
}

void USP_MenuButton::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (MainButton)
	{
		const bool bShouldHighlight = MainButton->HasKeyboardFocus() || MainButton->IsHovered();
		if (bShouldHighlight != bHighlighted)
		{
			ApplyHighlight(bShouldHighlight);
		}
	}
}

FReply USP_MenuButton::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	if (MainButton)
	{
		return FReply::Handled().SetUserFocus(MainButton->TakeWidget(), InFocusEvent.GetCause());
	}
	return Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
}

void USP_MenuButton::SetLabel(const FText& InLabel)
{
	ButtonLabel = InLabel;
	if (LabelText)
	{
		LabelText->SetText(ButtonLabel);
	}
}

void USP_MenuButton::FocusButton()
{
	if (!MainButton)
	{
		return;
	}

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		MainButton->SetUserFocus(PlayerController);
	}
	else
	{
		MainButton->SetKeyboardFocus();
	}
}

UWidget* USP_MenuButton::GetFocusTarget() const
{
	return MainButton;
}

void USP_MenuButton::HandleClicked()
{
	OnPressed.Broadcast();
}

void USP_MenuButton::HandleHovered()
{
	if (bFocusOnHover)
	{
		FocusButton();
	}
}

void USP_MenuButton::ApplyHighlight(bool bNewHighlighted)
{
	bHighlighted = bNewHighlighted;

	if (MainButton && bStylesReady)
	{
		MainButton->SetStyle(bHighlighted ? HighlightStyle : DefaultStyle);
	}
	if (LabelText)
	{
		LabelText->SetColorAndOpacity(bHighlighted ? HighlightedTextColor : TextColor);
	}
}
