// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_ScavengeScan_OlivierStan.generated.h"

/**
 * 
 */
UCLASS()
class OLIVIERSTANZOMBIERUNTIME_API UBTT_ScavengeScan_OlivierStan : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTT_ScavengeScan_OlivierStan();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector ScavengeLocationKey;

    UPROPERTY(EditAnywhere, Category = "Scan Settings")
    float RotationSpeed = 360.0f;

private:
    struct RotationValue
    {
        float TotalDegreesRotated;
        FRotator TargetRotation;
    };

    virtual uint16 GetInstanceMemorySize() const override { return sizeof(RotationValue); }
};
