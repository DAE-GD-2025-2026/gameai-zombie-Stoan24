// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Flee.generated.h"

/**
 * 
 */
UCLASS()
class GAMEAI_ZOMBIE_API UBTT_Flee : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTT_Flee();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Flee")
    float FleeDistance{ 600.f };

    UPROPERTY(EditAnywhere, Category = "Flee")
    float SafeDistance{ 800.f };

    UPROPERTY(EditAnywhere, Category = "Evade")
    float PredictionTime{ 1.5f };


    UPROPERTY(EditAnywhere, Category = "Flee")
    FBlackboardKeySelector ZombieActorKey;

private:
    FVector PredictZombiePosition(AActor* Zombie, APawn* Pawn) const;
};
