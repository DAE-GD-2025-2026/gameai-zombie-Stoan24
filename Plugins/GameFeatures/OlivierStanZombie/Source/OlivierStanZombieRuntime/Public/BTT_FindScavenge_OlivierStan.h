// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_FindScavenge_OlivierStan.generated.h"

/**
 * 
 */
UCLASS()
class OLIVIERSTANZOMBIERUNTIME_API UBTT_FindScavenge_OlivierStan : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTT_FindScavenge_OlivierStan();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector HouseActorKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector ScavengeLocationKey;
};
