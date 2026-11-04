#include "Arena/ArenaPathfinder.h"
#include "Arena/ArenaGrid.h"

bool FArenaPathfinder::ArePathConnected(
	const FArenaGrid& Grid,
	const FIntVector& Start,
	const FIntVector& Goal,
	const TArray<FIntVector>& ExtraBlockedCells)
{
	const FIntVector Size = Grid.GetSize();
	if (Size.X <= 0 || Size.Y <= 0)
	{
		return false;
	}

	auto IsBlocked = [&Grid, &ExtraBlockedCells](int32 X, int32 Y) -> bool
	{
		const FIntVector Ground(X, Y, 0);
		if (!Grid.IsInBounds(Ground) || Grid.IsMovementBlocked(Ground))
		{
			return true;
		}
		for (const FIntVector& Extra : ExtraBlockedCells)
		{
			if (Extra.Z == 0 && Extra.X == X && Extra.Y == Y)
			{
				return true;
			}
		}
		return false;
	};

	if (IsBlocked(Start.X, Start.Y) || IsBlocked(Goal.X, Goal.Y))
	{
		return false;
	}
	if (Start.X == Goal.X && Start.Y == Goal.Y)
	{
		return true;
	}

	TBitArray<> Visited(false, Size.X * Size.Y);
	TArray<FIntPoint> Queue;
	Queue.Reserve(Size.X * Size.Y);

	Queue.Add(FIntPoint(Start.X, Start.Y));
	Visited[Start.X + Size.X * Start.Y] = true;

	static const FIntPoint Directions[4] = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };

	for (int32 Head = 0; Head < Queue.Num(); ++Head)
	{
		const FIntPoint Current = Queue[Head];
		for (const FIntPoint& Direction : Directions)
		{
			const int32 NX = Current.X + Direction.X;
			const int32 NY = Current.Y + Direction.Y;
			if (NX < 0 || NY < 0 || NX >= Size.X || NY >= Size.Y)
			{
				continue;
			}

			const int32 Index = NX + Size.X * NY;
			if (Visited[Index] || IsBlocked(NX, NY))
			{
				continue;
			}
			if (NX == Goal.X && NY == Goal.Y)
			{
				return true;
			}

			Visited[Index] = true;
			Queue.Add(FIntPoint(NX, NY));
		}
	}

	return false;
}
