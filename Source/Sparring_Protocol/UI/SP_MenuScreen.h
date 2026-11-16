#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SP_MenuScreen.generated.h"

class USP_MenuButton;

/**
 * Base of every full screen menu (main menu, pause, death screen, leaderboard...).
 *  - keyboard / gamepad: the default button gets the focus when the screen opens,
 *    the arrows and the D-pad move between buttons, Escape / gamepad B goes back
 *  - screens can open another screen above them (settings, leaderboard); the focus
 *    comes back to the first one when the second closes
 *  - helpers to switch between "menu" input (cursor visible) and "game" input
 */
UCLASS(Abstract)
class SPARRING_PROTOCOL_API USP_MenuScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Puts the focus on the default button of this screen. */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void FocusDefault();

	/** Opens another screen above this one. Escape / Back on it brings the focus back here. */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	USP_MenuScreen* OpenChildScreen(TSubclassOf<USP_MenuScreen> ScreenClass, int32 ZOrder = 20);

	/** Removes the screen (and the one opened above it) and gives the focus back to its parent. */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	virtual void CloseScreen();

	/** Cursor visible, the UI receives the keys. bKeepGameInput: the game still gets its inputs (pause key). */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ActivateMenuInput(bool bKeepGameInput = false);

	/** Cursor hidden, back to playing. */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ActivateGameInput();

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** The widget focused when the screen opens. Null: the screen itself keeps the focus. */
	virtual UWidget* GetDefaultFocusWidget() const { return nullptr; }

	/** Escape / gamepad B. Return true if handled. */
	virtual bool HandleBack() { return false; }

	static bool IsBackKey(const FKey& Key);
	static bool IsAcceptKey(const FKey& Key);

	/** Close the child screen if one is open. */
	void CloseChildScreen();

	TWeakObjectPtr<USP_MenuScreen> ParentScreen;
	TWeakObjectPtr<USP_MenuScreen> ChildScreen;
};
