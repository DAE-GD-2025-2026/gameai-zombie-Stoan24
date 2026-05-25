// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Flee.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Survivor/SurvivorPawn.h"

UBTT_Flee::UBTT_Flee()
{
    NodeName = TEXT("Flee");

    bNotifyTick = false;
}

EBTNodeResult::Type UBTT_Flee::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    auto* ZombieActor = Cast<AActor>(BB->GetValueAsObject(ZombieActorKey.SelectedKeyName));
    if (!ZombieActor) return EBTNodeResult::Failed;

    auto Pawn = Controller->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    auto Survivor = Cast<ASurvivorPawn>(Pawn);
    if (!Survivor) return EBTNodeResult::Failed;

    float Distance = FVector::Dist(Pawn->GetActorLocation(), ZombieActor->GetActorLocation());
    if (Distance >= SafeDistance)
    {
        BB->ClearValue(ZombieActorKey.SelectedKeyName);
        Survivor->StopRunning();
        return EBTNodeResult::Succeeded;
    }

    Survivor->StartRunning();


    FVector PureAwayDir = (Pawn->GetActorLocation() - ZombieActor->GetActorLocation()).GetSafeNormal();
    FVector PredictedPos = PredictZombiePosition(ZombieActor, Pawn);
    FVector FleeDir = (Pawn->GetActorLocation() - PredictedPos).GetSafeNormal();

    if (FVector::DotProduct(FleeDir, PureAwayDir) < 0.f)
    {
        FleeDir = PureAwayDir;
    }

    FleeDir.Z = 0.f;
    FleeDir.Normalize();

    FVector ProposedTarget = Pawn->GetActorLocation() + (FleeDir * FleeDistance);

    TArray<FVector> PathPoints = Survivor->CalculatePath(ProposedTarget);

    if (PathPoints.Num() <= 1)
    {
        FVector BestAlternativeTarget = ProposedTarget;
        float BestPathScore = -1.f;

        TArray<float> TestAngles = { -45.f, 45.f, -90.f, 90.f };

        for (float Angle : TestAngles)
        {
            FVector RotatedDir = FleeDir.RotateAngleAxis(Angle, FVector::UpVector);
            FVector TestTarget = Pawn->GetActorLocation() + (RotatedDir * FleeDistance);

            TArray<FVector> TestPath = Survivor->CalculatePath(TestTarget);

            if (TestPath.Num() > 1)
            {
                float TargetDistToZombie = FVector::Dist(TestPath.Last(), ZombieActor->GetActorLocation());

                if (TargetDistToZombie > BestPathScore)
                {
                    BestPathScore = TargetDistToZombie;
                    BestAlternativeTarget = TestPath.Last();
                    PathPoints = TestPath;
                }
            }
        }
    }

    FVector FinalMoveTarget = ProposedTarget;
    if (PathPoints.Num() > 1)
    {
        FinalMoveTarget = PathPoints.Last();

        for (int32 i = 0; i < PathPoints.Num() - 1; ++i)
        {
            DrawDebugLine(GetWorld(), PathPoints[i], PathPoints[i + 1], FColor::Red, false, 0.4f, 0, 5.0f);
        }
        DrawDebugSphere(GetWorld(), FinalMoveTarget, 30.f, 12, FColor::Red, false, 0.4f, 0, 1.f);
    }

    Controller->MoveToLocation(FinalMoveTarget, -1.f, false, true, true, true, nullptr, false);
    return EBTNodeResult::Succeeded;
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