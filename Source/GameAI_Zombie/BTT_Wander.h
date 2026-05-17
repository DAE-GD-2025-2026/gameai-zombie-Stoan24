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
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;


    UPROPERTY(EditAnywhere, Category = "Wander")
    float CircleDistance{ 200.f };


    UPROPERTY(EditAnywhere, Category = "Wander")
    float CircleRadius{ 100.f };


    UPROPERTY(EditAnywhere, Category = "Wander")
    float AngleChangeSpeed{ 2.f };

private:
    float CurrentAngle{ 0.f };
};
