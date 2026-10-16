#include "Misc/AutomationTest.h"
#include "Arena/ArenaGrid.h"
#include "Arena/ArenaPathfinder.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	FArenaGrid MakeGrid(int32 X, int32 Y, int32 Z)
	{
		FArenaGrid Grid;
		Grid.Initialize(FIntVector(X, Y, Z), 200.f, FVector::ZeroVector);
		return Grid;
	}

	void BlockColumn(FArenaGrid& Grid, int32 X, int32 FromY, int32 ToY, int32 Z = 0)
	{
		for (int32 Y = FromY; Y <= ToY; ++Y)
		{
			Grid.SetMovementBlocked(FIntVector(X, Y, Z), true);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FArenaPathfinderBasicTest,
	"SparringProtocol.Arena.Pathfinder.Basic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FArenaPathfinderBasicTest::RunTest(const FString& Parameters)
{
	FArenaGrid Grid = MakeGrid(5, 5, 1);
	const TArray<FIntVector> NoExtra;

	TestTrue(TEXT("Open grid is connected"),
		FArenaPathfinder::ArePathConnected(Grid, FIntVector(0, 0, 0), FIntVector(4, 4, 0), NoExtra));
	TestTrue(TEXT("Start == Goal is connected"),
		FArenaPathfinder::ArePathConnected(Grid, FIntVector(2, 2, 0), FIntVector(2, 2, 0), NoExtra));
	TestFalse(TEXT("Out of bounds start is not connected"),
		FArenaPathfinder::ArePathConnected(Grid, FIntVector(-1, 0, 0), FIntVector(4, 4, 0), NoExtra));

	Grid.SetMovementBlocked(FIntVector(0, 0, 0), true);
	TestFalse(TEXT("Blocked start is not connected"),
		FArenaPathfinder::ArePathConnected(Grid, FIntVector(0, 0, 0), FIntVector(4, 4, 0), NoExtra));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FArenaPathfinderWallTest,
	"SparringProtocol.Arena.Pathfinder.Walls",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FArenaPathfinderWallTest::RunTest(const FString& Parameters)
{
	const FIntVector Start(0, 0, 0);
	const FIntVector Goal(4, 0, 0);
	const TArray<FIntVector> NoExtra;

	// Wall across X = 2 with one gap at Y = 4.
	FArenaGrid Gap = MakeGrid(5, 5, 1);
	BlockColumn(Gap, 2, 0, 3);
	TestTrue(TEXT("A one-cell gap keeps the path open"),
		FArenaPathfinder::ArePathConnected(Gap, Start, Goal, NoExtra));

	// Hypothetically closing the gap must cut the path, without touching the grid.
	TArray<FIntVector> CloseGap;
	CloseGap.Add(FIntVector(2, 4, 0));
	TestFalse(TEXT("Simulated block of the gap cuts the path"),
		FArenaPathfinder::ArePathConnected(Gap, Start, Goal, CloseGap));
	TestTrue(TEXT("The simulation did not modify the grid"),
		FArenaPathfinder::ArePathConnected(Gap, Start, Goal, NoExtra));

	// Full wall.
	FArenaGrid Full = MakeGrid(5, 5, 1);
	BlockColumn(Full, 2, 0, 4);
	TestFalse(TEXT("A full wall cuts the path"),
		FArenaPathfinder::ArePathConnected(Full, Start, Goal, NoExtra));

	// Diagonal-only gap: cell (0,0) only touches the rest through its diagonal.
	FArenaGrid Diagonal = MakeGrid(5, 5, 1);
	Diagonal.SetMovementBlocked(FIntVector(1, 0, 0), true);
	Diagonal.SetMovementBlocked(FIntVector(0, 1, 0), true);
	TestFalse(TEXT("A diagonal gap is not passable"),
		FArenaPathfinder::ArePathConnected(Diagonal, FIntVector(0, 0, 0), FIntVector(4, 4, 0), NoExtra));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FArenaPathfinderLevelsTest,
	"SparringProtocol.Arena.Pathfinder.Levels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FArenaPathfinderLevelsTest::RunTest(const FString& Parameters)
{
	FArenaGrid Grid = MakeGrid(5, 5, 2);
	const TArray<FIntVector> NoExtra;

	// A full wall on level 1 only does not block ground movement.
	BlockColumn(Grid, 2, 0, 4, 1);
	TestTrue(TEXT("Level 1 blocks do not affect ground paths"),
		FArenaPathfinder::ArePathConnected(Grid, FIntVector(0, 0, 0), FIntVector(4, 0, 0), NoExtra));

	// Neither do simulated blocks above ground level.
	TArray<FIntVector> UpperExtra;
	for (int32 Y = 0; Y < 5; ++Y)
	{
		UpperExtra.Add(FIntVector(2, Y, 1));
	}
	TestTrue(TEXT("Simulated level 1 blocks are ignored"),
		FArenaPathfinder::ArePathConnected(Grid, FIntVector(0, 0, 0), FIntVector(4, 0, 0), UpperExtra));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
