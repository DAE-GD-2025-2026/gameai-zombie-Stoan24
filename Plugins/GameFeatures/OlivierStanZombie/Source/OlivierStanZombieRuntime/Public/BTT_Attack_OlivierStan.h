// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Attack_OlivierStan.generated.h"

/**
 * 
 */
UCLASS()
class OLIVIERSTANZOMBIERUNTIME_API UBTT_Attack_OlivierStan : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTT_Attack_OlivierStan();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector ZombieActorKey;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float MaxCombatRange = 800.f;
};
