#include "UI/SP_ScoreList.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/SP_UIStyle.h"

namespace
{
	UTextBlock* ScoreListMakeText(UWidgetTree* Tree, const FText& Text, const FSlateFontInfo& Font, const FLinearColor& Color)
	{
		UTextBlock* Block = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Block->SetText(Text);
		Block->SetFont(Font);
		Block->SetColorAndOpacity(FSlateColor(Color));
		return Block;
	}

	FText ScoreListTwoDigits(int32 Value)
	{
		return FText::FromString(FString::Printf(TEXT("%02d"), Value));
	}
}

TSharedRef<SWidget> USP_ScoreList::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildFrame();
	}

	TSharedRef<SWidget> Result = Super::RebuildWidget();
	RefreshRows();
	return Result;
}

void USP_ScoreList::BuildFrame()
{
	USizeBox* Root = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Root->SetWidthOverride(PanelWidth);
	WidgetTree->RootWidget = Root;

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Panel->SetBrush(SPUIStyle::RoundedBox(SPUIStyle::Surface().CopyWithNewOpacity(0.85f), SPUIStyle::Outline(), 8.f));
	Panel->SetPadding(FMargin(32.f, 24.f));
	Root->AddChild(Panel);

	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Panel->SetContent(Column);

	// Header: "HALL OF FAME" .......... "ROUNDS"
	UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UTextBlock* TitleText = ScoreListMakeText(WidgetTree, Title, SPUIStyle::BodyFont(22, TEXT("Bold"), 240), SPUIStyle::Violet());
	UHorizontalBoxSlot* TitleSlot = Header->AddChildToHorizontalBox(TitleText);
	TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	TitleSlot->SetVerticalAlignment(VAlign_Bottom);

	UTextBlock* RoundsText = ScoreListMakeText(WidgetTree, NSLOCTEXT("SPMenu", "RoundsHeader", "ROUNDS"), SPUIStyle::BodyFont(14, TEXT("SemiBold"), 200), SPUIStyle::TextMuted());
	UHorizontalBoxSlot* RoundsSlot = Header->AddChildToHorizontalBox(RoundsText);
	RoundsSlot->SetVerticalAlignment(VAlign_Bottom);

	UVerticalBoxSlot* HeaderSlot = Column->AddChildToVerticalBox(Header);
	HeaderSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 14.f));

	Rows = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Column->AddChildToVerticalBox(Rows);
}

void USP_ScoreList::SetEntries(const TArray<FSPLeaderboardEntry>& InEntries, int32 InHighlightIndex)
{
	Entries = InEntries;
	HighlightIndex = InHighlightIndex;
	RefreshRows();
}

void USP_ScoreList::RefreshRows()
{
	if (!Rows || !WidgetTree)
	{
		return;
	}

	Rows->ClearChildren();

	for (int32 Index = 0; Index < MaxRows; ++Index)
	{
		const bool bHasEntry = Entries.IsValidIndex(Index);
		const bool bHighlighted = bHasEntry && Index == HighlightIndex;

		FLinearColor NameColor = bHasEntry ? SPUIStyle::TextLight() : SPUIStyle::Outline();
		FLinearColor ScoreColor = (Index == 0 && bHasEntry) ? SPUIStyle::Gold() : NameColor;
		if (bHighlighted)
		{
			NameColor = SPUIStyle::Green();
			ScoreColor = SPUIStyle::Green();
		}

		// Line background: a green outline for the player's score, nothing otherwise.
		UBorder* Line = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Line->SetBrush(bHighlighted
			? SPUIStyle::RoundedBox(SPUIStyle::Green().CopyWithNewOpacity(0.08f), SPUIStyle::Green(), 6.f, 2.f)
			: SPUIStyle::RoundedBox(FLinearColor::Transparent, FLinearColor::Transparent, 6.f, 0.f));
		Line->SetPadding(FMargin(18.f, 8.f));

		UHorizontalBox* Cells = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		Line->SetContent(Cells);

		USizeBox* RankBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		RankBox->SetWidthOverride(56.f);
		RankBox->AddChild(ScoreListMakeText(WidgetTree, ScoreListTwoDigits(Index + 1), SPUIStyle::BodyFont(24, TEXT("Bold")), SPUIStyle::TextMuted()));
		Cells->AddChildToHorizontalBox(RankBox);

		const FText NameText = bHasEntry ? FText::FromString(Entries[Index].Name) : FText::FromString(TEXT("---"));
		UHorizontalBoxSlot* NameSlot = Cells->AddChildToHorizontalBox(ScoreListMakeText(WidgetTree, NameText, SPUIStyle::BodyFont(24, TEXT("Bold"), 200), NameColor));
		NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		const FText ScoreText = bHasEntry ? ScoreListTwoDigits(Entries[Index].Rounds) : FText::FromString(TEXT("--"));
		Cells->AddChildToHorizontalBox(ScoreListMakeText(WidgetTree, ScoreText, SPUIStyle::BodyFont(24, TEXT("Bold")), ScoreColor));

		UVerticalBoxSlot* LineSlot = Rows->AddChildToVerticalBox(Line);
		LineSlot->SetPadding(FMargin(0.f, 2.f));
	}
}
