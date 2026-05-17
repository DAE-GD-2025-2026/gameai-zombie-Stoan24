// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Wander.h"
#include "AIController.h"
#include "NavigationSystem.h"

UBTT_Wander::UBTT_Wander()
{
    NodeName = TEXT("Wander");
}

EBTNodeResult::Type UBTT_Wander::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    auto Pawn = Controller->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    FNavLocation RandomLocation;
    auto* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
    if (NavSys && NavSys->GetRandomReachablePointInRadius(Pawn->GetActorLocation(), WanderRadius, RandomLocation))
    {
        Controller->MoveToLocation(RandomLocation.Location, 50.f);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
