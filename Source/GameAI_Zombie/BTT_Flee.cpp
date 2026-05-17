// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Flee.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTT_Flee::UBTT_Flee()
{
    NodeName = TEXT("Flee");

    bNotifyTick = true;
}

EBTNodeResult::Type UBTT_Flee::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    auto Pawn = Controller->GetPawn();
    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!Pawn || !BB) return EBTNodeResult::Failed;

    FVector ZombieLocation = BB->GetValueAsVector(ZombieLocationKey.SelectedKeyName);
    FVector FleeDir = (Pawn->GetActorLocation() - ZombieLocation).GetSafeNormal();
    FVector FleeTarget = Pawn->GetActorLocation() + FleeDir * FleeDistance;

    Controller->MoveToLocation(FleeTarget, 50.f);
    return EBTNodeResult::InProgress;
}

void UBTT_Flee::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    auto Pawn = Controller->GetPawn();
    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!Pawn || !BB) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    FVector ZombieLocation;
    auto* ZombieActor = Cast<AActor>(BB->GetValueAsObject(ZombieActorKey.SelectedKeyName));
    if (ZombieActor)
    {
        ZombieLocation = ZombieActor->GetActorLocation();
        BB->SetValueAsVector(ZombieLocationKey.SelectedKeyName, ZombieLocation);
    }
    else
    {
        ZombieLocation = BB->GetValueAsVector(ZombieLocationKey.SelectedKeyName);
    }

    float Distance = FVector::Dist(Pawn->GetActorLocation(), ZombieLocation);

    if (Distance >= SafeDistance)
    {
        BB->ClearValue(ZombieLocationKey.SelectedKeyName);
        BB->ClearValue(ZombieActorKey.SelectedKeyName);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    FVector FleeDir = (Pawn->GetActorLocation() - ZombieLocation).GetSafeNormal();
    FVector FleeTarget = Pawn->GetActorLocation() + FleeDir * FleeDistance;

    Controller->MoveToLocation(FleeTarget, 50.f);
}