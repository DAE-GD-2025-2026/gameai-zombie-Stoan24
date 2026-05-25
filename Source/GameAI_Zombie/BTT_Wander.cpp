// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Wander.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Survivor/SurvivorPawn.h"

UBTT_Wander::UBTT_Wander()
{
    NodeName = TEXT("Wander");

    bNotifyTick = true;
}

EBTNodeResult::Type UBTT_Wander::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    auto Pawn = Controller->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    auto* Survivor = Cast<ASurvivorPawn>(Pawn);
    if (!Survivor) return EBTNodeResult::Failed;


    CurrentAngle = FMath::RandRange(0.f, 2.f * PI);
    FVector Forward = Pawn->GetActorForwardVector();
    FVector CircleCenter = Pawn->GetActorLocation() + Forward * CircleDistance;

    FVector ProposedWanderTarget = CircleCenter + FVector(
        FMath::Cos(CurrentAngle) * CircleRadius,
        FMath::Sin(CurrentAngle) * CircleRadius,
        0.f
    );


    TArray<FVector> PathPoints = Survivor->CalculatePath(ProposedWanderTarget);


    if (PathPoints.Num() <= 1)
    {
        return EBTNodeResult::Failed;
    }


    FVector SafeWanderTarget = PathPoints.Last();

    Controller->MoveToLocation(SafeWanderTarget, 10.f, false, true, true, true, nullptr, false);

    return EBTNodeResult::Succeeded;
}

void UBTT_Wander::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}