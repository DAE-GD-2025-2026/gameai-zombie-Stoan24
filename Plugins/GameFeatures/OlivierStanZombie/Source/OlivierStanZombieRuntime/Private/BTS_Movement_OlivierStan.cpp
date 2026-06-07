// Fill out your copyright notice in the Description page of Project Settings.


#include "BTS_Movement_OlivierStan.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../StudentPerceptor.h"
#include "Navigation/PathFollowingComponent.h"

UBTS_Movement_OlivierStan::UBTS_Movement_OlivierStan()
{
    NodeName = "Context Steering Movement";

    bNotifyTick = true;
}

void UBTS_Movement_OlivierStan::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* Controller = OwnerComp.GetAIOwner();
    APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

    if (!Pawn || !BB) return;

    auto* MovementComp = Pawn->FindComponentByClass<UCharacterMovementComponent>();
    auto* Perceptor = Pawn->FindComponentByClass<UStudentPerceptor>();
    if (!MovementComp || !Perceptor) return;


    if (Controller->GetMoveStatus() != EPathFollowingStatus::Type::Moving) return;

    FVector CharacterLocation = Pawn->GetActorLocation();


    FVector TacticalTargetWorldPos = FVector::ZeroVector;
    bool bHasTacticalTarget = false;

    FVector ItemLocation = BB->GetValueAsVector(TEXT("ItemLocation"));
    if (!ItemLocation.IsZero())
    {
        TacticalTargetWorldPos = ItemLocation;
        bHasTacticalTarget = true;
    }
    else
    {
        FVector ScavengeLocation = BB->GetValueAsVector(TEXT("ScavengeLocation"));
        if (!ScavengeLocation.IsZero())
        {
            TacticalTargetWorldPos = ScavengeLocation;
            bHasTacticalTarget = true;
        }
        else
        {
            FVector ExplorePoint = BB->GetValueAsVector(TEXT("ExplorePoint"));
            if (!ExplorePoint.IsZero())
            {
                TacticalTargetWorldPos = ExplorePoint;
                bHasTacticalTarget = true;
            }
        }
    }


    FVector BasePathDirection = Controller->GetImmediateMoveDestination() - CharacterLocation;
    BasePathDirection.Z = 0.0f;
    BasePathDirection.Normalize();

    FVector TargetVelocity = BasePathDirection * MovementComp->MaxWalkSpeed;
    FVector TotalRepulsionForce = FVector::ZeroVector;


    const TArray<AActor*>& TrackedZombies = Perceptor->GetZombies();

    for (AActor* Zombie : TrackedZombies)
    {
        if (IsValid(Zombie))
        {
            FVector ZombieLocation = Zombie->GetActorLocation();
            float Distance = FVector::Dist2D(CharacterLocation, ZombieLocation);

            if (Distance < DangerRadius && Distance > 1.0f)
            {
                FVector PushDirection = CharacterLocation - ZombieLocation;
                PushDirection.Z = 0.0f;
                PushDirection.Normalize();

                if (bHasTacticalTarget)
                {
                    FVector TargetDirection = TacticalTargetWorldPos - CharacterLocation;
                    TargetDirection.Z = 0.0f;
                    TargetDirection.Normalize();

                    FVector FlankDirection = FVector::CrossProduct(PushDirection, FVector::UpVector);
                    if (FVector::DotProduct(FlankDirection, TargetDirection) < 0.0f)
                    {
                        FlankDirection *= -1.0f;
                    }
                    PushDirection = FMath::Lerp(PushDirection, FlankDirection, 0.4f);
                    PushDirection.Normalize();
                }

                float ProximityFactor = 1.0f - (Distance / DangerRadius);
                TotalRepulsionForce += PushDirection * (RepulsionStrength * ProximityFactor);
            }
        }
    }


    if (!TotalRepulsionForce.IsZero())
    {
        TargetVelocity += TotalRepulsionForce;
    }
    else if (!bHasTacticalTarget)
    {
        static float SpiralAngle = 0.f;
        SpiralAngle += DeltaSeconds * 2.0f;

        FVector SpiralOffset = FVector(FMath::Sin(SpiralAngle), FMath::Cos(SpiralAngle), 0.f);
        TargetVelocity += SpiralOffset * (MovementComp->MaxWalkSpeed * 0.35f);
    }

    TargetVelocity.Z = 0.0f;
    TargetVelocity = TargetVelocity.GetClampedToMaxSize(MovementComp->MaxWalkSpeed);


    if (auto* PathFollowingComp = Controller->GetPathFollowingComponent())
    {
        MovementComp->Velocity = FMath::VInterpTo(MovementComp->Velocity, TargetVelocity, DeltaSeconds, 8.0f);


        FVector AdjustedMovementDirection = MovementComp->Velocity;
        AdjustedMovementDirection.Normalize();

        FVector MoveVelocity = AdjustedMovementDirection * MovementComp->MaxWalkSpeed;


        PathFollowingComp->RequestMove(MoveVelocity, nullptr);
    }
}
