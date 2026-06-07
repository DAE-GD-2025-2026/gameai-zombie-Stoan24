// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FindExplorePoint_OlivierStan.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AC_HeatMap_OlivierStan.h"

UBTT_FindExplorePoint_OlivierStan::UBTT_FindExplorePoint_OlivierStan()
{
    NodeName = "Find Heatmap Exploration Point";

    ExplorationPointKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_FindExplorePoint_OlivierStan, ExplorationPointKey));
}

EBTNodeResult::Type UBTT_FindExplorePoint_OlivierStan::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!Controller || !BB) return EBTNodeResult::Failed;

    APawn* Pawn = Controller->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    //Get Heatmap location and give it to the BB
    UAC_HeatMap_OlivierStan* Heatmap = Pawn->FindComponentByClass<UAC_HeatMap_OlivierStan>();
    if (!Heatmap) return EBTNodeResult::Failed;


    FVector CurrentLocation = Pawn->GetActorLocation();

    if (BB->IsVectorValueSet(ExplorationPointKey.SelectedKeyName))
    {
        FVector ExistingTarget = BB->GetValueAsVector(ExplorationPointKey.SelectedKeyName);


        if (!ExistingTarget.IsZero())
        {
            if (FVector::DistSquared(CurrentLocation, ExistingTarget) > FMath::Square(500.f))
            {
                return EBTNodeResult::Succeeded;
            }
        }
    }

    FVector NewExploreTarget = Heatmap->FindNearestUnexploredLocation(Pawn->GetActorLocation());

    BB->SetValueAsVector(ExplorationPointKey.SelectedKeyName, NewExploreTarget);

    return EBTNodeResult::Succeeded;
}
