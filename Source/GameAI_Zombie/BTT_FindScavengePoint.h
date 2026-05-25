// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_FindScavengePoint.generated.h"

/**
 * 
 */
UCLASS()
class GAMEAI_ZOMBIE_API UBTT_FindScavengePoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTT_FindScavengePoint();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector HouseActorKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector ScavengeLocationKey;
};
