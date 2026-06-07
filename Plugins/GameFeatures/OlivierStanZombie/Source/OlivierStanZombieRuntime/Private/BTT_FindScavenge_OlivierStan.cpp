// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FindScavenge_OlivierStan.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTT_FindScavenge_OlivierStan::UBTT_FindScavenge_OlivierStan()
{
	NodeName = "Find Scavenge Point";
}

EBTNodeResult::Type UBTT_FindScavenge_OlivierStan::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    AActor* HouseActor = Cast<AActor>(BB->GetValueAsObject(HouseActorKey.SelectedKeyName));
    if (!HouseActor) return EBTNodeResult::Failed;

    //Get middle of house
    FVector Origin;
    FVector Extents;
    HouseActor->GetActorBounds(true, Origin, Extents);


    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return EBTNodeResult::Failed;

    FNavLocation TargetPoint;

    bool bSuccess = NavSys->ProjectPointToNavigation(Origin, TargetPoint, FVector(200.f, 200.f, 500.f));

    if (!bSuccess)
    {
        bSuccess = NavSys->GetRandomReachablePointInRadius(Origin, 150.f, TargetPoint);
    }

    if (bSuccess)
    {
        BB->SetValueAsVector(ScavengeLocationKey.SelectedKeyName, TargetPoint.Location);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
