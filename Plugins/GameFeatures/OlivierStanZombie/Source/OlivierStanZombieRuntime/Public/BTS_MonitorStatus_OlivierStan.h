// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_MonitorStatus_OlivierStan.generated.h"

/**
 * 
 */
UCLASS()
class OLIVIERSTANZOMBIERUNTIME_API UBTS_MonitorStatus_OlivierStan : public UBTService
{
	GENERATED_BODY()

public:
    UBTS_MonitorStatus_OlivierStan();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector NeedsWeaponKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector IsInventoryFullKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector NeededItemTypeKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector ZombieActorKey;
};
