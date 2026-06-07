// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_PickupItem_OlivierStan.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Survivor/SurvivorPawn.h"
#include "../StudentPerceptor.h"

UBTT_PickupItem_OlivierStan::UBTT_PickupItem_OlivierStan()
{
	NodeName = TEXT("Pickup Item");

	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_PickupItem_OlivierStan::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    auto* Item = Cast<ABaseItem>(BB->GetValueAsObject(ItemActorKey.SelectedKeyName));
    if (!Item) return EBTNodeResult::Failed;

    //Debug drawing
    DrawDebugLine(GetWorld(), Controller->GetPawn()->GetActorLocation(), Item->GetActorLocation(), FColor::Yellow, false, 0.4f, 0, 5.0f);

    DrawDebugSphere(GetWorld(), Item->GetActorLocation(), 30.f, 12, FColor::Yellow, false, 0.4f, 0, 1.f);

    Controller->MoveToLocation(Item->GetActorLocation());

    return EBTNodeResult::InProgress;
}

void UBTT_PickupItem_OlivierStan::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    auto* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn());
    auto* BB = OwnerComp.GetBlackboardComponent();
    auto* Item = Cast<ABaseItem>(BB->GetValueAsObject(ItemActorKey.SelectedKeyName));

    if (!Survivor || !BB) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }



    if (!Item || Item->GetItemType() == EItemType::Garbage)
    {
        BB->ClearValue(ItemActorKey.SelectedKeyName);
        BB->ClearValue(ItemLocationKey.SelectedKeyName);
        BB->SetValueAsBool(TEXT("IsPursuingItem"), false);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    auto* Inventory = Survivor->GetComponentByClass<UInventoryComponent>();
    if (!Inventory)
    {
	    FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    	return;
    }


    //Pickup when close enough
    float Distance = FVector::Dist2D(Survivor->GetActorLocation(), Item->GetActorLocation());
    UE_LOG(LogTemp, Warning, TEXT("Survivor Pos: %s | Item Pos: %s"), *Survivor->GetActorLocation().ToString(), *Item->GetActorLocation().ToString());


    if (Distance < Inventory->GetPickupRange())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
            FString::Printf(TEXT("Picking up: %s"), *Item->GetClass()->GetName()));

        const TArray<ABaseItem*>& CurrentInventory = Inventory->GetInventory();
        int Capacity = Inventory->GetInventoryCapacity();

        for (int i = 0; i < Capacity; i++)
        {
            if (i < CurrentInventory.Num() && CurrentInventory[i] == nullptr)
            {
                //Clear from memory
                if (auto* Perceptor = Survivor->FindComponentByClass<UStudentPerceptor>())
                {
                    Perceptor->ForgetPickedUpItem(Item);
                }

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

void UBTT_PickupItem_OlivierStan::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        BB->ClearValue(ItemLocationKey.SelectedKeyName);
    }
}
