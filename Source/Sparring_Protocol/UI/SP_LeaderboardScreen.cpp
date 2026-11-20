#include "UI/SP_LeaderboardScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "Score/SP_LeaderboardSubsystem.h"
#include "UI/SP_MenuButton.h"
#include "UI/SP_ScoreList.h"
#include "UI/SP_UIStyle.h"

USP_LeaderboardScreen::USP_LeaderboardScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MenuButtonClass = TSoftClassPtr<USP_MenuButton>(FSoftObjectPath(TEXT("/Game/UI/Widgets/WBP_MenuButton.WBP_MenuButton_C")));
}

TSharedRef<SWidget> USP_LeaderboardScreen::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildTree();
	}
	return Super::RebuildWidget();
}

void USP_LeaderboardScreen::BuildTree()
{
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	WidgetTree->RootWidget = Root;

	// Dark background over the main menu.
	UImage* Background = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Background->SetColorAndOpacity(SPUIStyle::Background().CopyWithNewOpacity(0.96f));
	UOverlaySlot* BackgroundSlot = Root->AddChildToOverlay(Background);
	BackgroundSlot->SetHorizontalAlignment(HAlign_Fill);
	BackgroundSlot->SetVerticalAlignment(VAlign_Fill);

	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UOverlaySlot* ColumnSlot = Root->AddChildToOverlay(Column);
	ColumnSlot->SetHorizontalAlignment(HAlign_Center);
	ColumnSlot->SetVerticalAlignment(VAlign_Center);

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Title->SetText(NSLOCTEXT("SPMenu", "LeaderboardTitle", "HALL OF FAME"));
	Title->SetFont(SPUIStyle::TitleFont(64));
	Title->SetColorAndOpacity(FSlateColor(SPUIStyle::Violet()));
	UVerticalBoxSlot* TitleSlot = Column->AddChildToVerticalBox(Title);
	TitleSlot->SetHorizontalAlignment(HAlign_Center);
	TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 32.f));

	ScoreList = WidgetTree->ConstructWidget<USP_ScoreList>(USP_ScoreList::StaticClass());
	ScoreList->MaxRows = USP_LeaderboardSubsystem::MaxEntries;
	ScoreList->PanelWidth = 620.f;
	ScoreList->Title = NSLOCTEXT("SPMenu", "LocalTop", "LOCAL TOP 10");
	UVerticalBoxSlot* ListSlot = Column->AddChildToVerticalBox(ScoreList);
	ListSlot->SetHorizontalAlignment(HAlign_Center);
	ListSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 32.f));

	if (UClass* ButtonClass = MenuButtonClass.LoadSynchronous())
	{
		BackButton = WidgetTree->ConstructWidget<USP_MenuButton>(ButtonClass);
		BackButton->ButtonLabel = NSLOCTEXT("SPMenu", "Back", "BACK");
		UVerticalBoxSlot* ButtonSlot = Column->AddChildToVerticalBox(BackButton);
		ButtonSlot->SetHorizontalAlignment(HAlign_Center);
	}
	else
	{
		UTextBlock* Hint = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Hint->SetText(NSLOCTEXT("SPMenu", "BackHint", "ESC  BACK"));
		Hint->SetFont(SPUIStyle::BodyFont(18, TEXT("SemiBold"), 160));
		Hint->SetColorAndOpacity(FSlateColor(SPUIStyle::TextMuted()));
		UVerticalBoxSlot* HintSlot = Column->AddChildToVerticalBox(Hint);
		HintSlot->SetHorizontalAlignment(HAlign_Center);
	}
}

void USP_LeaderboardScreen::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->SetLabel(NSLOCTEXT("SPMenu", "Back", "BACK"));
		BackButton->OnPressed.AddUniqueDynamic(this, &USP_LeaderboardScreen::HandleBackPressed);
	}

	if (ScoreList)
	{
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			if (const USP_LeaderboardSubsystem* Leaderboard = GameInstance->GetSubsystem<USP_LeaderboardSubsystem>())
			{
				ScoreList->SetEntries(Leaderboard->GetEntries(), INDEX_NONE);
			}
		}
	}
}

UWidget* USP_LeaderboardScreen::GetDefaultFocusWidget() const
{
	return BackButton;
}

bool USP_LeaderboardScreen::HandleBack()
{
	CloseScreen();
	return true;
}

void USP_LeaderboardScreen::HandleBackPressed()
{
	CloseScreen();
}
