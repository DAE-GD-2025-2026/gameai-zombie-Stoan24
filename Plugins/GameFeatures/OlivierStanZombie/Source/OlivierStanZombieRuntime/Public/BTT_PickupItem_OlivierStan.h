// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_PickupItem_OlivierStan.generated.h"

/**
 * 
 */
UCLASS()
class OLIVIERSTANZOMBIERUNTIME_API UBTT_PickupItem_OlivierStan : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTT_PickupItem_OlivierStan();
protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;


    UPROPERTY(EditAnywhere, Category = "Pickup")
    FBlackboardKeySelector ItemActorKey;
    UPROPERTY(EditAnywhere, Category = "Pickup")
    FBlackboardKeySelector ItemLocationKey;
};
