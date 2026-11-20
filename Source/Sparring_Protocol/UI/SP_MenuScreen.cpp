#include "UI/SP_MenuScreen.h"

#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "UI/SP_MenuButton.h"

void USP_MenuScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// The screen can hold the focus itself, and passes it to its default button.
	SetIsFocusable(true);
}

FReply USP_MenuScreen::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	UWidget* Target = GetDefaultFocusWidget();
	if (USP_MenuButton* Button = Cast<USP_MenuButton>(Target))
	{
		Target = Button->GetFocusTarget();
	}

	if (Target)
	{
		return FReply::Handled().SetUserFocus(Target->TakeWidget(), InFocusEvent.GetCause());
	}
	return Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
}

FReply USP_MenuScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (IsBackKey(InKeyEvent.GetKey()) && HandleBack())
	{
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void USP_MenuScreen::FocusDefault()
{
	APlayerController* PlayerController = GetOwningPlayer();
	UWidget* Target = GetDefaultFocusWidget();

	if (USP_MenuButton* Button = Cast<USP_MenuButton>(Target))
	{
		Button->FocusButton();
	}
	else if (Target)
	{
		PlayerController ? Target->SetUserFocus(PlayerController) : Target->SetKeyboardFocus();
	}
	else
	{
		PlayerController ? SetUserFocus(PlayerController) : SetKeyboardFocus();
	}
}

USP_MenuScreen* USP_MenuScreen::OpenChildScreen(TSubclassOf<USP_MenuScreen> ScreenClass, int32 ZOrder)
{
	if (!ScreenClass)
	{
		return nullptr;
	}

	CloseChildScreen();

	USP_MenuScreen* Screen = CreateWidget<USP_MenuScreen>(GetOwningPlayer(), ScreenClass);
	if (!Screen)
	{
		return nullptr;
	}

	Screen->ParentScreen = this;
	ChildScreen = Screen;
	Screen->AddToViewport(ZOrder);
	Screen->FocusDefault();
	return Screen;
}

void USP_MenuScreen::CloseChildScreen()
{
	if (USP_MenuScreen* Child = ChildScreen.Get())
	{
		Child->ParentScreen = nullptr;
		Child->CloseScreen();
	}
	ChildScreen = nullptr;
}

void USP_MenuScreen::CloseScreen()
{
	CloseChildScreen();
	RemoveFromParent();

	if (USP_MenuScreen* Parent = ParentScreen.Get())
	{
		Parent->ChildScreen = nullptr;
		Parent->FocusDefault();
	}
	ParentScreen = nullptr;
}

void USP_MenuScreen::ActivateMenuInput(bool bKeepGameInput)
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	if (bKeepGameInput)
	{
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(Mode);
	}
	else
	{
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(Mode);
	}
	PlayerController->SetShowMouseCursor(true);
}

void USP_MenuScreen::ActivateGameInput()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
}

bool USP_MenuScreen::IsBackKey(const FKey& Key)
{
	return Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right;
}

bool USP_MenuScreen::IsAcceptKey(const FKey& Key)
{
	return Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom;
}
