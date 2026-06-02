// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Flee_OlivierStan.generated.h"

/**
 * 
 */
UCLASS()
class OLIVIERSTANZOMBIERUNTIME_API UBTT_Flee_OlivierStan : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTT_Flee_OlivierStan();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

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

    float RepathTimer{ 0.f };
};
