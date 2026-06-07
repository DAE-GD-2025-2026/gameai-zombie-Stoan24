// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Flee_OlivierStan.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Survivor/SurvivorPawn.h"

UBTT_Flee_OlivierStan::UBTT_Flee_OlivierStan()
{
	NodeName = TEXT("Flee");

	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_Flee_OlivierStan::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    TickTask(OwnerComp, NodeMemory, 0.f);

    return EBTNodeResult::InProgress;
}

void UBTT_Flee_OlivierStan::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    auto* Controller = OwnerComp.GetAIOwner();
    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!Controller || !BB) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    auto* ZombieActor = Cast<AActor>(BB->GetValueAsObject(ZombieActorKey.SelectedKeyName));
    auto Pawn = Controller->GetPawn();
    auto* Survivor = Cast<ASurvivorPawn>(Pawn);
    if (!ZombieActor || !Pawn || !Survivor) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }


    float DistanceToZombie = FVector::Dist(Pawn->GetActorLocation(), ZombieActor->GetActorLocation());
    if (DistanceToZombie >= SafeDistance)
    {
        Survivor->StopRunning();
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }


    Survivor->StartRunning();



    //Locate Targets: Item --> Scavenge --> Explore
    FVector TargetWorldPos = FVector::ZeroVector;
    bool bHasTacticalTarget = false;

    uint8 CurrentTask = BB->GetValueAsEnum(TEXT("CurrentTask"));

    if (CurrentTask == static_cast<uint8>(ESurvivorTask::CollectItem))
    {
        FVector ItemLocation = BB->GetValueAsVector(TEXT("ItemLocation"));
        if (!ItemLocation.IsZero())
        {
	        TargetWorldPos = ItemLocation;
        	bHasTacticalTarget = true;
        }
    }
    else if (CurrentTask == static_cast<uint8>(ESurvivorTask::Scavenge))
    {
        FVector ScavengeLocation = BB->GetValueAsVector(TEXT("ScavengeLocation"));
        if (!ScavengeLocation.IsZero())
        {
	        TargetWorldPos = ScavengeLocation;
        	bHasTacticalTarget = true;
        }
    }
    else if (CurrentTask == static_cast<uint8>(ESurvivorTask::Explore))
    {
        FVector ExplorePoint = BB->GetValueAsVector(TEXT("ExplorePoint"));
        if (!ExplorePoint.IsZero())
        {
	        TargetWorldPos = ExplorePoint;
        	bHasTacticalTarget = true;
        }
    }

    //Arrival check
    if (bHasTacticalTarget)
    {
        float DistanceToTarget = FVector::Dist(Pawn->GetActorLocation(), TargetWorldPos);
        if (DistanceToTarget <= 175.f)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            return;
        }
    }

    //Apply Target
    FVector PureAwayDir = (Pawn->GetActorLocation() - ZombieActor->GetActorLocation()).GetSafeNormal();
    FVector PredictedPos = PredictZombiePosition(ZombieActor, Pawn);
    FVector FleeDir = (Pawn->GetActorLocation() - PredictedPos).GetSafeNormal();

    if (FVector::DotProduct(FleeDir, PureAwayDir) < 0.f)
    {
        FleeDir = PureAwayDir;
    }
    FleeDir.Z = 0.f;
    FleeDir.Normalize();


    //Default
    FVector FinalMoveTarget = Pawn->GetActorLocation() + (FleeDir * FleeDistance);
    TArray<FVector> PathPoints = Survivor->CalculatePath(FinalMoveTarget);


    if (bHasTacticalTarget)
    {
        //Calculate direct route toward Target
        TArray<FVector> TacticalPath = Survivor->CalculatePath(TargetWorldPos);

        if (TacticalPath.Num() > 1)
        {
            FVector NextPathCorner = TacticalPath[1];
            FVector DirToCorner = (NextPathCorner - Pawn->GetActorLocation()).GetSafeNormal();


            //Check if zombie is blocking the path
            FVector FlankDir = FMath::Lerp(FleeDir, DirToCorner, 0.5f).GetSafeNormal();

            FlankDir.Z = 0.f;
            FlankDir.Normalize();


            FVector FlankTarget = Pawn->GetActorLocation() + (FlankDir * FleeDistance);
            TArray<FVector> FlankPath = Survivor->CalculatePath(FlankTarget);

            if (FlankPath.Num() > 1)
            {
                PathPoints = FlankPath;
                FinalMoveTarget = FlankPath.Last();
            }
            else
            {
                if (TacticalPath.Num() > 0)
                {
                    PathPoints = TacticalPath;
                    FinalMoveTarget = TacticalPath.Last();
                }
            }
        }
    }

    //Debug draw
    if (PathPoints.Num() > 1)
    {
        FinalMoveTarget = PathPoints.Last();

        for (int i = 0; i < PathPoints.Num() - 1; ++i)
        {
            //Draws yellow for house/objective --> Red for Zombie
            FColor LineColor = bHasTacticalTarget ? FColor::Yellow : FColor::Red;
            DrawDebugLine(GetWorld(), PathPoints[i], PathPoints[i + 1], LineColor, false, 0.05f, 0, 5.0f);
        }
        DrawDebugSphere(GetWorld(), FinalMoveTarget, 30.f, 12, FColor::Red, false, 0.05f, 0, 1.f);
    }

    //Don't do MoveToLocation every frame
    static FVector LastIssuedTarget = FVector::ZeroVector;

    if (FVector::DistSquared(LastIssuedTarget, FinalMoveTarget) > FMath::Square(50.f))
    {
        Controller->MoveToLocation(FinalMoveTarget, -1.f, false, true, true, true, nullptr, false);
        LastIssuedTarget = FinalMoveTarget;
    }
}

void UBTT_Flee_OlivierStan::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        BB->ClearValue(ZombieActorKey.SelectedKeyName);
    }
}

FVector UBTT_Flee_OlivierStan::PredictZombiePosition(AActor* Zombie, APawn* Pawn) const
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
