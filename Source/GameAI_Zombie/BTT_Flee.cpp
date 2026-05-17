// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Flee.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/PawnMovementComponent.h"

UBTT_Flee::UBTT_Flee()
{
    NodeName = TEXT("Flee");

    bNotifyTick = true;
}

EBTNodeResult::Type UBTT_Flee::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    auto* BB = OwnerComp.GetBlackboardComponent();

    auto* ZombieActor = Cast<AActor>(BB->GetValueAsObject(ZombieActorKey.SelectedKeyName));
    if (!ZombieActor) return EBTNodeResult::Failed;

    auto Pawn = Controller->GetPawn();
    
    FVector PredictedPos = PredictZombiePosition(ZombieActor, Pawn);
    FVector FleeDir = (Pawn->GetActorLocation() - PredictedPos).GetSafeNormal();
    FVector FleeTarget = Pawn->GetActorLocation() + FleeDir * FleeDistance;

    Controller->MoveToLocation(FleeTarget);
    return EBTNodeResult::InProgress;
}

void UBTT_Flee::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    auto Pawn = Controller->GetPawn();
    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!Pawn || !BB) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }


    auto* ZombieActor = Cast<AActor>(BB->GetValueAsObject(ZombieActorKey.SelectedKeyName));
    if (!ZombieActor)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    float Distance = FVector::Dist(Pawn->GetActorLocation(), ZombieActor->GetActorLocation());

    if (Distance >= SafeDistance)
    {
        BB->ClearValue(ZombieActorKey.SelectedKeyName);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }


    FVector PredictedPos = PredictZombiePosition(ZombieActor, Pawn);
    FVector FleeDir = (Pawn->GetActorLocation() - PredictedPos).GetSafeNormal();
    FVector FleeTarget = Pawn->GetActorLocation() + FleeDir * FleeDistance;

    Controller->MoveToLocation(FleeTarget);
}

FVector UBTT_Flee::PredictZombiePosition(AActor* Zombie, APawn* Pawn) const
{
    FVector ZombieVelocity = FVector::ZeroVector;
    if (auto* ZombiePawn = Cast<APawn>(Zombie))
    {
        if (auto* MoveComp = ZombiePawn->GetMovementComponent())
        {
            ZombieVelocity = MoveComp->Velocity;
        }
    }


    float Distance = FVector::Dist(Pawn->GetActorLocation(), Zombie->GetActorLocation());
    float DynamicPrediction = PredictionTime * (Distance / FleeDistance);
    DynamicPrediction = FMath::Clamp(DynamicPrediction, 0.5f, PredictionTime * 2.f);


    return Zombie->GetActorLocation() + ZombieVelocity * DynamicPrediction;
}