// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Wander.h"
#include "AIController.h"
#include "NavigationSystem.h"

UBTT_Wander::UBTT_Wander()
{
    NodeName = TEXT("Wander");

    bNotifyTick = true;
}

EBTNodeResult::Type UBTT_Wander::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    CurrentAngle = FMath::RandRange(0.f, 2.f * PI);
    return EBTNodeResult::InProgress;
}

void UBTT_Wander::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    auto Pawn = Controller->GetPawn();
    if (!Pawn) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }


    //Randomize angle
    CurrentAngle += FMath::RandRange(-AngleChangeSpeed, AngleChangeSpeed) * DeltaSeconds;


    FVector Forward = Pawn->GetActorForwardVector();
    FVector CircleCenter = Pawn->GetActorLocation() + Forward * CircleDistance;


    FVector WanderTarget = CircleCenter + FVector(
        FMath::Cos(CurrentAngle) * CircleRadius,
        FMath::Sin(CurrentAngle) * CircleRadius,
        0.f
    );

    Controller->MoveToLocation(WanderTarget, 10.f);
}