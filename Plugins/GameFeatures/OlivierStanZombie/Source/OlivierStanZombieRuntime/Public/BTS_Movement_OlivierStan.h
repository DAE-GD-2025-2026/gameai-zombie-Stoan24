// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_Movement_OlivierStan.generated.h"

/**
 * 
 */
UCLASS()
class OLIVIERSTANZOMBIERUNTIME_API UBTS_Movement_OlivierStan : public UBTService
{
	GENERATED_BODY()

public:
    UBTS_Movement_OlivierStan();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Steering")
    float DangerRadius = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Steering")
    float RepulsionStrength = 800.f;
};
