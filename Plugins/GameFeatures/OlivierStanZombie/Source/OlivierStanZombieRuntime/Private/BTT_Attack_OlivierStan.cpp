// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Attack_OlivierStan.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/InventoryComponent.h"
#include "Survivor/SurvivorPawn.h"
#include "Items/BaseItem.h"

UBTT_Attack_OlivierStan::UBTT_Attack_OlivierStan()
{
    NodeName = "Attack";
}

EBTNodeResult::Type UBTT_Attack_OlivierStan::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* Controller = OwnerComp.GetAIOwner();
    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!Controller || !BB) return EBTNodeResult::Failed;

    auto* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn());
    auto* Zombie = Cast<AActor>(BB->GetValueAsObject(ZombieActorKey.SelectedKeyName));
    if (!Survivor || !Zombie) return EBTNodeResult::Failed;

    float Distance = FVector::Dist(Survivor->GetActorLocation(), Zombie->GetActorLocation());
    if (Distance > MaxCombatRange) return EBTNodeResult::Failed;

    auto* Inventory = Survivor->GetComponentByClass<UInventoryComponent>();
    if (!Inventory) return EBTNodeResult::Failed;


    ABaseItem* WeaponToUse = nullptr;
    for (ABaseItem* Item : Inventory->GetInventory())
    {
        if (Item && (Item->GetItemType() == EItemType::Pistol || Item->GetItemType() == EItemType::Shotgun))
        {
            if (Item->GetValue() > 0)
            {
                WeaponToUse = Item;
                break;
            }
        }
    }

    if (!WeaponToUse) return EBTNodeResult::Failed;


    Controller->StopMovement();
    FVector LookDir = (Zombie->GetActorLocation() - Survivor->GetActorLocation()).GetSafeNormal2D();
    Survivor->SetActorRotation(LookDir.Rotation());


    WeaponToUse->UseItem(*Survivor);

    DrawDebugLine(GetWorld(), Survivor->GetActorLocation(), Zombie->GetActorLocation(), FColor::Orange, false, 0.2f, 0, 4.0f);

    return EBTNodeResult::Succeeded;
}
