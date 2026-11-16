#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"

/**
 * Shared look of the menus: the colors of the mockup (dark violet club, violet = decor,
 * green = what the player can use) and the fonts imported in Content/UI/Fonts.
 * Used by the screens built in code (leaderboard, hall of fame, settings).
 */
namespace SPUIStyle
{
	inline FLinearColor Hex(uint8 R, uint8 G, uint8 B, float Alpha = 1.f)
	{
		FLinearColor Color = FLinearColor::FromSRGBColor(FColor(R, G, B));
		Color.A = Alpha;
		return Color;
	}

	inline FLinearColor Background() { return Hex(0x0B, 0x06, 0x14); }
	inline FLinearColor Surface() { return Hex(0x1C, 0x10, 0x30); }
	inline FLinearColor Outline() { return Hex(0x3A, 0x21, 0x60); }
	inline FLinearColor Violet() { return Hex(0xA1, 0x3B, 0xFF); }
	inline FLinearColor Green() { return Hex(0x39, 0xFF, 0x88); }
	inline FLinearColor Gold() { return Hex(0xFF, 0xE9, 0xA8); }
	inline FLinearColor Danger() { return Hex(0xFF, 0x3B, 0x5C); }
	inline FLinearColor TextLight() { return Hex(0xE8, 0xDD, 0xF7); }
	inline FLinearColor TextMuted() { return Hex(0xB9, 0xA6, 0xDA); }

	/** Barlow Condensed. Typeface: "Medium", "SemiBold" or "Bold". */
	SPARRING_PROTOCOL_API FSlateFontInfo BodyFont(int32 Size, FName Typeface = TEXT("SemiBold"), int32 LetterSpacing = 0);

	/** Monoton, the neon sign font (titles only). */
	SPARRING_PROTOCOL_API FSlateFontInfo TitleFont(int32 Size, int32 LetterSpacing = 0);

	/** Rounded rectangle with an outline (the "dance floor tile" look). */
	SPARRING_PROTOCOL_API FSlateBrush RoundedBox(const FLinearColor& Fill, const FLinearColor& OutlineColor, float Radius, float OutlineWidth = 1.f);
}
