// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_PickNextHouse_OlivierStan.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "OlivierStanZombieRuntime/StudentPerceptor.h"

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


    UStudentPerceptor* Perceptor = Pawn->FindComponentByClass<UStudentPerceptor>();
    if (!Perceptor) return EBTNodeResult::Failed;


    if (!Perceptor->HasVillageHouses())
    {
        BB->SetValueAsBool(HasVillageTargetKey.SelectedKeyName, false);
        BB->ClearValue(HouseActorKey.SelectedKeyName);
        return EBTNodeResult::Succeeded;
    }


    AActor* NextHouse = Perceptor->PopNextVillageHouse();
    BB->SetValueAsObject(HouseActorKey.SelectedKeyName, NextHouse);

    return EBTNodeResult::Succeeded;
}
