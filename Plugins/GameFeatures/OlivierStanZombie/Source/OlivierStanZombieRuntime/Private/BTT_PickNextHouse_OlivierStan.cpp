// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_PickNextHouse_OlivierStan.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "OlivierStanZombieRuntime/StudentPerceptorOlivierStan.h"

UBTT_PickNextHouse_OlivierStan::UBTT_PickNextHouse_OlivierStan()
{
    NodeName = "Pick Next House from Village";


    HasVillageTargetKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_PickNextHouse_OlivierStan, HasVillageTargetKey));
    HouseActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_PickNextHouse_OlivierStan, HouseActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTT_PickNextHouse_OlivierStan::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!Controller || !BB) return EBTNodeResult::Failed;

    APawn* Pawn = Controller->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    UStudentPerceptorOlivierStan* Perceptor = Pawn->FindComponentByClass<UStudentPerceptorOlivierStan>();
    if (!Perceptor) return EBTNodeResult::Failed;


    //Clear old house
    if (Cast<AActor>(BB->GetValueAsObject(HouseActorKey.SelectedKeyName)))
    {
        BB->ClearValue(HouseActorKey.SelectedKeyName);
    }

    //Find houses in memory
    if (!Perceptor->HasVillageHouses())
    {
        BB->SetValueAsBool(HasVillageTargetKey.SelectedKeyName, false);
        return EBTNodeResult::Failed;
    }


    AActor* NextHouse = Perceptor->PopNextVillageHouse();
    
    if (IsValid(NextHouse))
    {
        BB->SetValueAsObject(HouseActorKey.SelectedKeyName, NextHouse);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Succeeded;
}
