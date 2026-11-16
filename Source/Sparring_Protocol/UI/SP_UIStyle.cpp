#include "UI/SP_UIStyle.h"

#include "Styling/CoreStyle.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	UObject* LoadCachedFont(TWeakObjectPtr<UObject>& Cache, const TCHAR* Path)
	{
		if (!Cache.IsValid())
		{
			Cache = FSoftObjectPath(Path).TryLoad();
		}
		return Cache.Get();
	}
}

FSlateFontInfo SPUIStyle::BodyFont(int32 Size, FName Typeface, int32 LetterSpacing)
{
	static TWeakObjectPtr<UObject> Cache;

	FSlateFontInfo Info;
	if (UObject* Font = LoadCachedFont(Cache, TEXT("/Game/UI/Fonts/F_BarlowCondensed.F_BarlowCondensed")))
	{
		Info = FSlateFontInfo(Font, Size, Typeface);
	}
	else
	{
		Info = FCoreStyle::GetDefaultFontStyle("Bold", Size);
	}
	Info.LetterSpacing = LetterSpacing;
	return Info;
}

FSlateFontInfo SPUIStyle::TitleFont(int32 Size, int32 LetterSpacing)
{
	static TWeakObjectPtr<UObject> Cache;

	FSlateFontInfo Info;
	if (UObject* Font = LoadCachedFont(Cache, TEXT("/Game/UI/Fonts/F_Monoton.F_Monoton")))
	{
		Info = FSlateFontInfo(Font, Size);
	}
	else
	{
		Info = FCoreStyle::GetDefaultFontStyle("Bold", Size);
	}
	Info.LetterSpacing = LetterSpacing;
	return Info;
}

FSlateBrush SPUIStyle::RoundedBox(const FLinearColor& Fill, const FLinearColor& OutlineColor, float Radius, float OutlineWidth)
{
	FSlateBrush Brush;
	Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
	Brush.TintColor = FSlateColor(Fill);
	Brush.OutlineSettings = FSlateBrushOutlineSettings(FVector4(Radius, Radius, Radius, Radius), FSlateColor(OutlineColor), OutlineWidth);
	Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
	return Brush;
}
