// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Wander.generated.h"

/**
 * 
 */
UCLASS()
class GAMEAI_ZOMBIE_API UBTT_Wander : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTT_Wander();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, Category = "Wander")
    float WanderRadius{ 800.f };
};
