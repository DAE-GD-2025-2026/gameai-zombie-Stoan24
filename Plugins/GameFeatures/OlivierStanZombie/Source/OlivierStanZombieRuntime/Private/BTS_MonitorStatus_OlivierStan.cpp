// Fill out your copyright notice in the Description page of Project Settings.


#include "BTS_MonitorStatus_OlivierStan.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Survivor/SurvivorPawn.h"
#include "Common/InventoryComponent.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "Items/BaseItem.h"

UBTS_MonitorStatus_OlivierStan::UBTS_MonitorStatus_OlivierStan()
{
	NodeName = "Monitor Status and Inventory";
	Interval = 0.5f;
}

void UBTS_MonitorStatus_OlivierStan::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return;

    auto* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn());
    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!Survivor || !BB) return;

    auto* Inventory = Survivor->GetComponentByClass<UInventoryComponent>();
    auto* HealthComp = Survivor->GetComponentByClass<UHealthComponent>();
    auto* StaminaComp = Survivor->GetComponentByClass<UStaminaComponent>();

    if (!Inventory || !HealthComp || !StaminaComp) return;

    EItemType HighestPriorityNeed = EItemType::Garbage;

    if (HealthComp->GetHealth() <= 6)
    {
        HighestPriorityNeed = EItemType::Medkit;
    }
    else if (StaminaComp->GetCurrentStamina() < 3.0f)
    {
        HighestPriorityNeed = EItemType::Food;
    }
    else
    {
        int32 PistolCount = 0;
        int32 ShotgunCount = 0;

        for (ABaseItem* Item : Inventory->GetInventory())
        {
            if (Item != nullptr)
            {
                if (Item->GetItemType() == EItemType::Pistol && Item->GetValue() > 0) PistolCount++;
                if (Item->GetItemType() == EItemType::Shotgun && Item->GetValue() > 0) ShotgunCount++;
            }
        }

        if (PistolCount == 0)
        {
            HighestPriorityNeed = EItemType::Pistol;
        }
        else if (ShotgunCount == 0)
        {
            HighestPriorityNeed = EItemType::Shotgun;
        }
    }

    BB->SetValueAsEnum(TEXT("NeededItemType"), static_cast<uint8>(HighestPriorityNeed));

    //Health & Stamina
    //TArray<ABaseItem*> Items = Inventory->GetInventory();

    //if (HealthComp && HealthComp->GetHealth() < 0.4f)
    //{
    //    for (int32 i = 0; i < Items.Num(); ++i)
    //    {
    //        if (Items[i] && Items[i]->GetItemType() == EItemType::Medkit)
    //        {
    //            Items[i]->UseItem(*Survivor);
    //            Items[i] = nullptr;
    //            GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Low Health, used Medkit"));
    //            break;
    //        }
    //    }
    //}

    //if (StaminaComp && StaminaComp->GetCurrentStamina() < 0.3f)
    //{
    //    for (int32 i = 0; i < Items.Num(); ++i)
    //    {
    //        if (Items[i] && Items[i]->GetItemType() == EItemType::Food)
    //        {
    //            Items[i]->UseItem(*Survivor);
    //            Items[i] = nullptr;
    //            GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Low Stamina, ate Food"));
    //            break;
    //        }
    //    }
    //}

    ////Create Ideal Loadout
    //int32 PistolCount = 0;
    //int32 ShotgunCount = 0;
    //int32 TotalItems = 0;

    //for (ABaseItem* Item : Items)
    //{
    //    if (Item != nullptr)
    //    {
    //        TotalItems++;
    //        if (Item->GetItemType() == EItemType::Pistol) PistolCount++;
    //        if (Item->GetItemType() == EItemType::Shotgun) ShotgunCount++;
    //    }
    //}

    //bool HasNoWeapons = (PistolCount < 1 || ShotgunCount < 1);

    //bool HasAmmo = false;
    //for (ABaseItem* Item : Items)
    //{
    //    if (Item && (Item->GetItemType() == EItemType::Pistol || Item->GetItemType() == EItemType::Shotgun))
    //    {
    //        if (Item->GetValue() > 0)
    //        {
    //            HasAmmo = true;
    //            break;
    //        }
    //    }
    //}


    //bool bNeedsWeaponState = HasNoWeapons || !HasAmmo;
    //bool bIsInventoryFullState = (TotalItems >= Inventory->GetInventoryCapacity());


    //if (bNeedsWeaponState)
    //{
    //    BB->SetValueAsBool(NeedsWeaponsKey.SelectedKeyName, true);
    //}
    //else
    //{
    //    BB->ClearValue(NeedsWeaponsKey.SelectedKeyName);
    //}

    //if (bIsInventoryFullState)
    //{
    //    BB->SetValueAsBool(IsInventoryFullKey.SelectedKeyName, true);
    //}
    //else
    //{
    //    BB->ClearValue(IsInventoryFullKey.SelectedKeyName);
    //}


}
