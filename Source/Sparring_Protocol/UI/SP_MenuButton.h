#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"
#include "SP_MenuButton.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSPOnMenuButtonPressed);

/**
 * Parent class of WBP_MenuButton (the "dance floor tile").
 * The Blueprint keeps the look (MainButton style, LabelText font); this class adds:
 *  - the same highlight for the mouse AND the keyboard / gamepad focus (arrows, D-pad)
 *  - hovering with the mouse moves the focus, so only one tile is lit at a time
 *  - OnPressed, fired by a click, Enter or the gamepad A button
 */
UCLASS()
class SPARRING_PROTOCOL_API USP_MenuButton : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Menu Button")
	void SetLabel(const FText& InLabel);

	/** Gives the keyboard / gamepad focus to this button. */
	UFUNCTION(BlueprintCallable, Category = "Menu Button")
	void FocusButton();

	UFUNCTION(BlueprintPure, Category = "Menu Button")
	bool IsHighlighted() const { return bHighlighted; }

	/** The widget that really takes the focus (the inner button). */
	UWidget* GetFocusTarget() const;

	/** Text of the button. The screens set it from code. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu Button")
	FText ButtonLabel = NSLOCTEXT("SPMenu", "DefaultButtonLabel", "BUTTON");

	/** False: hovering with the mouse does not take the focus (used while typing a name). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Button")
	bool bFocusOnHover = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu Button")
	FSlateColor TextColor = FSlateColor(FLinearColor::FromSRGBColor(FColor(0xE8, 0xDD, 0xF7)));

	/** Text color on the lit (green) tile. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu Button")
	FSlateColor HighlightedTextColor = FSlateColor(FLinearColor::FromSRGBColor(FColor(0x0B, 0x06, 0x14)));

	UPROPERTY(BlueprintAssignable, Category = "Menu Button")
	FSPOnMenuButtonPressed OnPressed;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MainButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LabelText;

private:
	UFUNCTION()
	void HandleClicked();

	UFUNCTION()
	void HandleHovered();

	void ApplyHighlight(bool bNewHighlighted);

	FButtonStyle DefaultStyle;
	FButtonStyle HighlightStyle;
	bool bHighlighted = false;
	bool bStylesReady = false;
};
