// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_AttackZombie.generated.h"

/**
 * 
 */
UCLASS()
class GAMEAI_ZOMBIE_API UBTT_AttackZombie : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTT_AttackZombie();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector ZombieActorKey;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float MaxCombatRange = 800.f;
};
