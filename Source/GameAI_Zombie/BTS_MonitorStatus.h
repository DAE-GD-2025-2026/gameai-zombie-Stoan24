// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_MonitorStatus.generated.h"

/**
 * 
 */
UCLASS()
class GAMEAI_ZOMBIE_API UBTS_MonitorStatus : public UBTService
{
	GENERATED_BODY()

public:
    UBTS_MonitorStatus();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector NeedsWeaponsKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector NeedsSuppliesKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector IsInventoryFullKey;
};
