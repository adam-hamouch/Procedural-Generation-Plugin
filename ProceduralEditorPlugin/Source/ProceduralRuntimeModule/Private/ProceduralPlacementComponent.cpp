#include "ProceduralPlacementComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Async/Async.h"
#include "Async/ParallelFor.h"
#include "DrawDebugHelpers.h"

UProceduralPlacementComponent::UProceduralPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UProceduralPlacementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UProceduralPlacementComponent::Generate()
{
	if (!Mesh || !Spline) return;

	if (Spline->ISMComp)
		Spline->ISMComp->ClearInstances();
	else
	{
		Spline->ISMComp	 =
		NewObject<UInstancedStaticMeshComponent>(GetOwner());
		Spline->ISMComp->SetupAttachment(GetOwner()->GetRootComponent());
		Spline->ISMComp->RegisterComponent();
		Spline->ISMComp->SetUsingAbsoluteLocation(false);
	}

	Spline->ISMComp->SetStaticMesh(Mesh);  
	GenerateFunction();
}

void UProceduralPlacementComponent::GenerateFunction()
{
	Positions.Empty();
	CacheSpline();
	
	PoissonDiskAlgo(Positions);
	ApplyPoints(Positions);
}

void UProceduralPlacementComponent::ApplyPoints(const TArray<FVector>& Points)
{
	Positions = Points;

	ProjectPoints();

	for (const FVector& Pos : Positions)
	{
		Spline->ISMComp->AddInstanceWorldSpace(FTransform(Pos));
	}
}

// Generates evenly distributed points using Poisson Disk Sampling.
// Ensures a minimum distance (Spacing) between points to avoid clustering.

void UProceduralPlacementComponent::PoissonDiskAlgo(TArray<FVector>& Points)
{
	FRandomStream Random(Seed);

	FVector Min = GetMinPoint();
	FVector Max = GetMaxPoint();

	float CellSIze = Spacing / FMath::Sqrt(2.f);

	const int32 GridWidth  = FMath::CeilToInt((Max.X - Min.X) / CellSIze);
	const int32 GridHeight = FMath::CeilToInt((Max.Y - Min.Y) / CellSIze);
	TArray<FGridCell> Grids;
	
	Grids.SetNum(GridWidth);
	
	for (int32 X = 0; X < GridWidth; X++)
	{
		Grids[X].PointsIndex.Init(-1, GridHeight);
	}

	TArray<FVector> ActivePoints;
	
	FVector FirstPoint = SplinePoints[0];
	FirstPoint.Z = 0.f;

	ActivePoints.Add(FirstPoint);
	Points.Add(FirstPoint);
	
	while (ActivePoints.Num() > 0)
	{
		int32 Index = Random.RandRange(0, ActivePoints.Num() - 1);
		FVector Current = ActivePoints[Index];
		bool bFound = false;

		for (int32 i = 0; i < 20; i++)
		{
			float Angle = Random.FRandRange(0, TWO_PI);
			float Dist  = Random.FRandRange(Spacing, Spacing * 2);

			FVector Candidate =
				Current +
				FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Dist;

			if (IsValid(Candidate, Points, Max, Min, CellSIze, Grids))
			{
				if (IsInside(Candidate))
				{
					Points.Add(Candidate);
					ActivePoints.Add(Candidate);

					const FVector Local = Candidate - Min;
					float CX = FMath::FloorToInt(Local.X / CellSIze);
					float CY = FMath::FloorToInt(Local.Y / CellSIze);

					Grids[CX].PointsIndex[CY] = Points.Num() - 1;
					bFound = true;
					break;
				}
			}
		}

		if (!bFound)
		{
			ActivePoints.RemoveAt(Index);
		}
	}
}

// Projects generated points onto the world geometry using line traces.
// This allows points to conform to terrain elevation.

void UProceduralPlacementComponent::ProjectPoints()
{
	for (int i = 0; i < Positions.Num(); ++i)
	{
		FHitResult Hit;
		FVector Start = Positions[i] + FVector(0,0,5000);
		FVector End   = Positions[i] - FVector(0,0,5000);

		if (GetWorld()->LineTraceSingleByChannel(
			Hit, Start, End, ECC_WorldStatic))
		{
			Positions[i] = Hit.Location;
		}
	}
}


// Checks whether a candidate point respects the minimum spacing constraint
// by inspecting neighboring grid cells only (O(1) average complexity).

bool UProceduralPlacementComponent::IsValid(FVector Candidate, TArray<FVector>& Points, FVector Max, FVector Min, float CellSIze, TArray<FGridCell> Grids)
{
	if (Candidate.X < Min.X || Candidate.X > Max.X ||
		Candidate.Y < Min.Y || Candidate.Y > Max.Y)
	{
		return false;
	}

	const FVector Local = Candidate - Min;

	float CX = FMath::FloorToInt(Local.X / CellSIze);
	float CY = FMath::FloorToInt(Local.Y / CellSIze);

	if (!Grids.IsValidIndex(CX) ||
		!Grids[CX].PointsIndex.IsValidIndex(CY))
	{
		return false;
	}

	for (int32 X = FMath::Max(0, CX - 2); X <= FMath::Min(CX + 2, Grids.Num() - 1); X++)
	{
		for (int32 Y = FMath::Max(0, CY - 2); Y <= FMath::Min(CY + 2, Grids[0].PointsIndex.Num() - 1); Y++)
		{
			int32 Idx = Grids[X].PointsIndex[Y];
			if (Idx != -1 &&
				FVector::DistSquared(Points[Idx], Candidate) < Spacing * Spacing)
			{
				return false;
			}
		}
	}

	return true;
}

// Check if the point is in the spline area
// Basic Raycasting algorithm

void UProceduralPlacementComponent::CacheSpline()
{
	SplinePoints.Empty();

	int32 N = Spline->GetNumberOfSplinePoints();
	for (int i = 0; i < N; i++)
	{
		SplinePoints.Add(
			Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World)
		);
	}
}

bool UProceduralPlacementComponent::IsInside(FVector Candidate)
{
	int N = SplinePoints.Num();
	bool Inside = false;

	for (int i = 0; i < N; i++)
	{
		FVector P1 = SplinePoints[i];
		FVector P2 = SplinePoints[(i + 1) % N];

		if (FMath::IsNearlyEqual(P1.Y, P2.Y))
			continue;

		bool yCheck = (P1.Y > Candidate.Y) != (P2.Y > Candidate.Y);

		double xIntersect =
			(P2.X - P1.X) * (Candidate.Y - P1.Y)
			/ (P2.Y - P1.Y)
			+ P1.X;

		if (yCheck && Candidate.X < xIntersect)
		{
			Inside = !Inside;
		}
	}

	return Inside;
}

// Get Max/Min points

FVector UProceduralPlacementComponent::GetMinPoint()
{
	FVector MinPoint = SplinePoints[0];
	
	float CurrentX = SplinePoints[0].X;
	float CurrentY = SplinePoints[0].Y;
	
	for (int i = 1; i < SplinePoints.Num(); ++i)
	{
		if (SplinePoints[i].X < CurrentX)
		{
			MinPoint.X = SplinePoints[i].X;
			CurrentX = MinPoint.X;
		}

		if (SplinePoints[i].Y < CurrentY)
		{
			MinPoint.Y = SplinePoints[i].Y;
			CurrentY = MinPoint.Y;
		}
	}
	return MinPoint;
}

FVector UProceduralPlacementComponent::GetMaxPoint()
{
	FVector MaxPoint = SplinePoints[0];
	float CurrentX = SplinePoints[0].X;
	float CurrentY = SplinePoints[0].Y;
	
	for (int i = 1; i < SplinePoints.Num(); ++i)
	{
		if (SplinePoints[i].X > CurrentX)
		{
			MaxPoint.X = SplinePoints[i].X;
			CurrentX = MaxPoint.X;
		}

		if (SplinePoints[i].Y > CurrentY)
		{
			MaxPoint.Y = SplinePoints[i].Y;
			CurrentY = MaxPoint.Y;
		}
	}
	return MaxPoint;
}
