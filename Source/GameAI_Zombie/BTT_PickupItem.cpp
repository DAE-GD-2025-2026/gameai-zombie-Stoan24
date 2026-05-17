// Fill out your copyright notice in the Description page of Project Settings.

// BTTask_PickupItem.cpp
#include "BTT_PickupItem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Survivor/SurvivorPawn.h"

#include "Engine/Engine.h"

UBTT_PickupItem::UBTT_PickupItem()
{
    NodeName = TEXT("Pickup Item");

    bNotifyTick = true;
}

EBTNodeResult::Type UBTT_PickupItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    auto* Item = Cast<ABaseItem>(BB->GetValueAsObject(ItemActorKey.SelectedKeyName));
    if (!Item) return EBTNodeResult::Failed;


    Controller->MoveToLocation(Item->GetActorLocation());
    
    return EBTNodeResult::InProgress;
}

void UBTT_PickupItem::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    auto* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn());
    auto* BB = OwnerComp.GetBlackboardComponent();
    auto* Item = Cast<ABaseItem>(BB->GetValueAsObject(ItemActorKey.SelectedKeyName));

    if (!Survivor || !BB) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }


    if (!Item)
    {
        BB->ClearValue(ItemActorKey.SelectedKeyName);
        BB->ClearValue(ItemLocationKey.SelectedKeyName);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    auto* Inventory = Survivor->GetComponentByClass<UInventoryComponent>();
    if (!Inventory) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }



    float Distance = FVector::Dist(Survivor->GetActorLocation(), Item->GetActorLocation());

    if (Distance <= Inventory->GetPickupRange())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
            FString::Printf(TEXT("Picking up: %s"), *Item->GetClass()->GetName()));

        for (int i = 0; i < Inventory->GetInventoryCapacity(); i++)
        {
            if (Inventory->GetInventory()[i] == nullptr)
            {
                Inventory->GrabItem(i, Item);
                BB->ClearValue(ItemActorKey.SelectedKeyName);
                BB->ClearValue(ItemLocationKey.SelectedKeyName);
                BB->SetValueAsBool(TEXT("IsPursuingItem"), false);
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                return;
            }
        }

        //Inventory full
        BB->ClearValue(ItemActorKey.SelectedKeyName);
        BB->ClearValue(ItemLocationKey.SelectedKeyName);
        BB->SetValueAsBool(TEXT("IsPursuingItem"), false);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }
}