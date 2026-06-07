// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_ScavengeScan_OlivierStan.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

UBTT_ScavengeScan_OlivierStan::UBTT_ScavengeScan_OlivierStan()
{
    NodeName = "Scavenge 360 Scan";

    bNotifyTick = true;
}

EBTNodeResult::Type UBTT_ScavengeScan_OlivierStan::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return EBTNodeResult::Failed;
    }


    RotationValue* rotationValues = reinterpret_cast<RotationValue*>(NodeMemory);
    rotationValues->TotalDegreesRotated = 0.0f;

    rotationValues->TargetRotation = AIController->GetPawn()->GetActorRotation();

    return EBTNodeResult::InProgress;
}

void UBTT_ScavengeScan_OlivierStan::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;

    if (!Pawn || !BB)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    //Rotation
    RotationValue* rotationValues = reinterpret_cast<RotationValue*>(NodeMemory);

    float DegreesToRotateThisFrame = RotationSpeed * DeltaSeconds;
    rotationValues->TotalDegreesRotated += DegreesToRotateThisFrame;


    rotationValues->TargetRotation.Yaw += DegreesToRotateThisFrame;
    rotationValues->TargetRotation.Yaw = FRotator::ClampAxis(rotationValues->TargetRotation.Yaw);

    FRotator NewRotation = rotationValues->TargetRotation;
    NewRotation.Pitch = 0.0f;
    NewRotation.Roll = 0.0f;
    Pawn->SetActorRotation(NewRotation);

    //Check if agent did full circle
    if (rotationValues->TotalDegreesRotated >= 360.0f)
    {
        BB->ClearValue(ScavengeLocationKey.SelectedKeyName);

        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

void UBTT_ScavengeScan_OlivierStan::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        BB->ClearValue(ScavengeLocationKey.SelectedKeyName);
    }
}
