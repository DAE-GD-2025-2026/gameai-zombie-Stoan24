// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AC_HeatMap_OlivierStan.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OLIVIERSTANZOMBIERUNTIME_API UAC_HeatMap_OlivierStan : public UActorComponent
{
	GENERATED_BODY()

public:	

	UAC_HeatMap_OlivierStan();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Exploration")
	FVector FindNearestUnexploredLocation(FVector CurrentLocation, float MaxSearchRadius = 10000.f);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heatmap Configuration")
	float CellSize = 200.f;

private:
	TMap<FVector2D, int> HeatmapGrid;
	FVector2D LastRegisteredGrid;

	FVector2D WorldToGrid(FVector Location) const;
	FVector GridToWorld(FVector2D GridIdx) const;
};
