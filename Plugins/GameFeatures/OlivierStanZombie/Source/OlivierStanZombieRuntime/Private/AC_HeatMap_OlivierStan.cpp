// Fill out your copyright notice in the Description page of Project Settings.


#include "AC_HeatMap_OlivierStan.h"

// Sets default values for this component's properties
UAC_HeatMap_OlivierStan::UAC_HeatMap_OlivierStan()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called every frame
void UAC_HeatMap_OlivierStan::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (AActor* Owner = GetOwner())
	{
		FVector2D CurrentGrid = WorldToGrid(Owner->GetActorLocation());
		HeatmapGrid.FindOrAdd(CurrentGrid)++;
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
	int32 SearchRingRadius = FMath::CeilToInt(MaxSearchRadius / CellSize);

	float BestDistance = MAX_FLT;
	FVector2D BestGridIndex = CenterGrid;
	bool bFoundUnexplored = false;


	//Spiral outward
	for (int32 Radius = 1; Radius <= SearchRingRadius; ++Radius)
	{
		for (int32 x = -Radius; x <= Radius; ++x)
		{
			for (int32 y = -Radius; y <= Radius; ++y)
			{
				if (FMath::Abs(x) == Radius || FMath::Abs(y) == Radius)
				{
					FVector2D CheckGrid = CenterGrid + FVector2D(x, y);

					if (!HeatmapGrid.Contains(CheckGrid) || HeatmapGrid[CheckGrid] == 0)
					{
						FVector TargetWorldPos = GridToWorld(CheckGrid);
						float DistSq = FVector::DistSquared(CurrentLocation, TargetWorldPos);

						if (DistSq < BestDistance)
						{
							BestDistance = DistSq;
							BestGridIndex = CheckGrid;
							bFoundUnexplored = true;
						}
					}
				}
			}
		}
		if (bFoundUnexplored) break;
	}

	FVector ResultWorldLocation = GridToWorld(BestGridIndex);
	ResultWorldLocation.Z = CurrentLocation.Z;
	return ResultWorldLocation;
}
