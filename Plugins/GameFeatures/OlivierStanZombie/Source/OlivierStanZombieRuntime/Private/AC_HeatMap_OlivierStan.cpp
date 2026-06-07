// Fill out your copyright notice in the Description page of Project Settings.


#include "AC_HeatMap_OlivierStan.h"


UAC_HeatMap_OlivierStan::UAC_HeatMap_OlivierStan()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called every frame
void UAC_HeatMap_OlivierStan::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (AActor* Owner = GetOwner())
	{
        FVector WorldPos = Owner->GetActorLocation();
        UE_LOG(LogTemp, Warning, TEXT("World Pos: X=%.1f Y=%.1f"), WorldPos.X, WorldPos.Y);

		FVector2D CurrentGrid = WorldToGrid(Owner->GetActorLocation());

		UE_LOG(LogTemp, Warning, TEXT("Heatmap cell registered: X=%.0f Y=%.0f"), CurrentGrid.X, CurrentGrid.Y);

		if (CurrentGrid != LastRegisteredGrid)
		{
			HeatmapGrid.FindOrAdd(CurrentGrid)++;

			LastRegisteredGrid = CurrentGrid;
		}
	}
}

FVector2D UAC_HeatMap_OlivierStan::WorldToGrid(FVector Location) const
{
	return FVector2D(FMath::FloorToInt(Location.X / CellSize), FMath::FloorToInt(Location.Y / CellSize));
}

FVector UAC_HeatMap_OlivierStan::GridToWorld(FVector2D GridIdx) const
{
	return FVector(GridIdx.X * CellSize + (CellSize * 0.5f), GridIdx.Y * CellSize + (CellSize * 0.5f), 0.f);
}

FVector UAC_HeatMap_OlivierStan::FindNearestUnexploredLocation(FVector CurrentLocation, float MaxSearchRadius)
{
    FVector2D CenterGrid = WorldToGrid(CurrentLocation);
    int MaxSearchRings = FMath::CeilToInt(MaxSearchRadius / CellSize);

    TArray<FVector2D> Candidates;

    for (int X = CenterGrid.X - MaxSearchRings; X <= CenterGrid.X + MaxSearchRings; X++)
    {
        for (int Y = CenterGrid.Y - MaxSearchRings; Y <= CenterGrid.Y + MaxSearchRings; Y++)
        {
            FVector2D Cell(X, Y);
            if (FVector2D::Distance(CenterGrid, Cell) <= MaxSearchRings)
            {
                if (!HeatmapGrid.Contains(Cell) || HeatmapGrid[Cell] == 0)
                {
                    Candidates.Add(Cell);
                }
            }
        }
    }

    if (Candidates.IsEmpty()) return CurrentLocation;

    //Pick random unvisited cell
    FVector2D Chosen = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
    FVector Result = GridToWorld(Chosen);
    Result.Z = CurrentLocation.Z;
    return Result;
}
