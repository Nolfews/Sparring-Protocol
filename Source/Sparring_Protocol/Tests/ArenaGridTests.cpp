#include "Misc/AutomationTest.h"
#include "Arena/ArenaGrid.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FArenaGridBoundsTest,
	"SparringProtocol.Arena.Grid.Bounds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FArenaGridBoundsTest::RunTest(const FString& Parameters)
{
	FArenaGrid Grid;
	Grid.Initialize(FIntVector(4, 3, 2), 200.f, FVector(100.f, 200.f, 0.f));

	TestTrue(TEXT("Origin cell is in bounds"), Grid.IsInBounds(FIntVector(0, 0, 0)));
	TestTrue(TEXT("Last cell is in bounds"), Grid.IsInBounds(FIntVector(3, 2, 1)));
	TestFalse(TEXT("X == SizeX is out of bounds"), Grid.IsInBounds(FIntVector(4, 0, 0)));
	TestFalse(TEXT("Y == SizeY is out of bounds"), Grid.IsInBounds(FIntVector(0, 3, 0)));
	TestFalse(TEXT("Z == SizeZ is out of bounds"), Grid.IsInBounds(FIntVector(0, 0, 2)));
	TestFalse(TEXT("Negative X is out of bounds"), Grid.IsInBounds(FIntVector(-1, 0, 0)));

	TestTrue(TEXT("Out of bounds cell reports OutOfBounds"),
		Grid.GetState(FIntVector(10, 10, 10)) == EArenaCellState::OutOfBounds);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FArenaGridStateTest,
	"SparringProtocol.Arena.Grid.State",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FArenaGridStateTest::RunTest(const FString& Parameters)
{
	FArenaGrid Grid;
	Grid.Initialize(FIntVector(4, 3, 2), 200.f, FVector::ZeroVector);

	TestTrue(TEXT("Cells start Free"), Grid.GetState(FIntVector(1, 1, 1)) == EArenaCellState::Free);

	Grid.SetState(FIntVector(1, 1, 1), EArenaCellState::Occupied);
	TestTrue(TEXT("SetState is readable"), Grid.GetState(FIntVector(1, 1, 1)) == EArenaCellState::Occupied);
	TestTrue(TEXT("Neighbour cell is untouched"), Grid.GetState(FIntVector(1, 1, 0)) == EArenaCellState::Free);
	TestTrue(TEXT("Other X is untouched"), Grid.GetState(FIntVector(2, 1, 1)) == EArenaCellState::Free);

	// Must not crash or corrupt anything.
	Grid.SetState(FIntVector(99, 0, 0), EArenaCellState::Occupied);
	Grid.SetState(FIntVector(-1, 0, 0), EArenaCellState::Occupied);

	// Every cell has a distinct storage slot: occupy one by one and count.
	FArenaGrid Counter;
	Counter.Initialize(FIntVector(4, 3, 2), 200.f, FVector::ZeroVector);
	for (int32 X = 0; X < 4; ++X)
	{
		for (int32 Y = 0; Y < 3; ++Y)
		{
			for (int32 Z = 0; Z < 2; ++Z)
			{
				Counter.SetState(FIntVector(X, Y, Z), EArenaCellState::Occupied);
			}
		}
	}
	int32 OccupiedCount = 0;
	for (int32 X = 0; X < 4; ++X)
	{
		for (int32 Y = 0; Y < 3; ++Y)
		{
			for (int32 Z = 0; Z < 2; ++Z)
			{
				OccupiedCount += (Counter.GetState(FIntVector(X, Y, Z)) == EArenaCellState::Occupied) ? 1 : 0;
			}
		}
	}
	TestEqual(TEXT("All 24 cells addressable"), OccupiedCount, 24);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FArenaGridConversionTest,
	"SparringProtocol.Arena.Grid.Conversion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FArenaGridConversionTest::RunTest(const FString& Parameters)
{
	const FVector Origin(100.f, 200.f, 0.f);
	FArenaGrid Grid;
	Grid.Initialize(FIntVector(4, 3, 2), 200.f, Origin);

	const FVector FirstCenter = Grid.CellToWorld(FIntVector(0, 0, 0));
	TestTrue(TEXT("Cell (0,0,0) center is Origin + half a cell"),
		FirstCenter.Equals(Origin + FVector(100.f, 100.f, 100.f)));

	// Round trip: WorldToCell(CellToWorld(c)) == c for every cell.
	bool bRoundTripOk = true;
	for (int32 X = 0; X < 4; ++X)
	{
		for (int32 Y = 0; Y < 3; ++Y)
		{
			for (int32 Z = 0; Z < 2; ++Z)
			{
				const FIntVector Cell(X, Y, Z);
				bRoundTripOk &= (Grid.WorldToCell(Grid.CellToWorld(Cell)) == Cell);
			}
		}
	}
	TestTrue(TEXT("CellToWorld / WorldToCell round trip"), bRoundTripOk);

	// Just below the origin must floor to -1, not truncate to 0.
	TestTrue(TEXT("Locations below the origin floor to negative cells"),
		Grid.WorldToCell(Origin - FVector(1.f, 1.f, 1.f)) == FIntVector(-1, -1, -1));

	// Cell borders: 199.9 is still the first cell, 200 starts the second.
	TestTrue(TEXT("Just inside the first cell"),
		Grid.WorldToCell(Origin + FVector(199.9f, 0.f, 0.f)).X == 0);
	TestTrue(TEXT("Exactly on the border starts the next cell"),
		Grid.WorldToCell(Origin + FVector(200.f, 0.f, 0.f)).X == 1);

	const FBox Bounds = Grid.GetWorldBounds();
	TestTrue(TEXT("World bounds min is the origin"), Bounds.Min.Equals(Origin));
	TestTrue(TEXT("World bounds max matches size * cell size"),
		Bounds.Max.Equals(Origin + FVector(800.f, 600.f, 400.f)));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
