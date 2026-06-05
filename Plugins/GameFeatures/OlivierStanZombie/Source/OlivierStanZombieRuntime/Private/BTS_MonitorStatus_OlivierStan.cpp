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

    const auto* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return;

    auto* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn());
    auto* BB = OwnerComp.GetBlackboardComponent();
    if (!Survivor || !BB) return;

    auto* Inventory = Survivor->GetComponentByClass<UInventoryComponent>();
    auto* HealthComp = Survivor->GetComponentByClass<UHealthComponent>();
    auto* StaminaComp = Survivor->GetComponentByClass<UStaminaComponent>();

    if (!Inventory || !HealthComp || !StaminaComp) return;

    //Health & Stamina

    TArray<ABaseItem*> Items = Inventory->GetInventory();

    if (HealthComp->GetHealth() <= 6)
    {
        for (int i = 0; i < Items.Num(); ++i)
        {
            if (Items[i] && Items[i]->GetItemType() == EItemType::Medkit)
            {
                Inventory->UseItem(i);
                Inventory->RemoveItem(i);

                GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Low Health --> used Medkit"));
                break;
            }
        }
    }

    if (StaminaComp->GetCurrentStamina() < 3.0f)
    {
        for (int i = 0; i < Items.Num(); ++i)
        {
            if (Items[i] && Items[i]->GetItemType() == EItemType::Food)
            {
                Inventory->UseItem(i);
                Inventory->RemoveItem(i);
                GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Low Stamina --> ate Food"));
                break;
            }
        }
    }

    //Inventory Management

    int PistolCount = 0;
    int ShotgunCount = 0;
    int FoodCount = 0;
    int MedkitCount = 0;
    int TotalItems = 0;

    int PistolWithAmmoCount = 0;
    int ShotgunWithAmmoCount = 0;

    for (int i = 0; i < Items.Num(); ++i)
    {
        ABaseItem* Item = Items[i];

        if (Item != nullptr)
        {
            bool bIsSpentPistol = (Item->GetItemType() == EItemType::Pistol && Item->GetValue() <= 0);
            bool bIsSpentShotgun = (Item->GetItemType() == EItemType::Shotgun && Item->GetValue() <= 0);

            if (bIsSpentPistol || bIsSpentShotgun)
            {
                Inventory->RemoveItem(i);
                GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, TEXT("Removed Weapon --> no ammo left"));
                continue;
            }

            TotalItems++;

            if (Item->GetItemType() == EItemType::Pistol)
            {
                PistolCount++;
                if (Item->GetValue() > 0) PistolWithAmmoCount++;
            }
            if (Item->GetItemType() == EItemType::Shotgun)
            {
                ShotgunCount++;
                if (Item->GetValue() > 0) ShotgunWithAmmoCount++;
            }

            if (Item->GetItemType() == EItemType::Food) FoodCount++;
            if (Item->GetItemType() == EItemType::Medkit) MedkitCount++;
        }
    }

    //Combat
    if (PistolWithAmmoCount == 0 || ShotgunWithAmmoCount == 0)
    {
        BB->SetValueAsBool(NeedsWeaponsKey.SelectedKeyName, true);
    }
    else
    {
        BB->ClearValue(NeedsWeaponsKey.SelectedKeyName);
    }

    //Active needs
    EItemType HighestPriorityNeed = EItemType::Garbage;

    if (PistolCount == 0)
    {
        HighestPriorityNeed = EItemType::Pistol;
    }
    else if (HealthComp->GetHealth() <= 6 && MedkitCount == 0)
    {
        HighestPriorityNeed = EItemType::Medkit;
    }
    else if (StaminaComp->GetCurrentStamina() < 3.0f && FoodCount == 0)
    {
        HighestPriorityNeed = EItemType::Food;
    }
    else if (ShotgunCount == 0)
    {
        HighestPriorityNeed = EItemType::Shotgun;
    }
    

    BB->SetValueAsEnum(NeededItemTypeKey.SelectedKeyName, static_cast<uint8>(HighestPriorityNeed));


    //Filter
    if (TotalItems >= Inventory->GetInventoryCapacity())
    {
        BB->SetValueAsBool(IsInventoryFullKey.SelectedKeyName, true);
    }
    else
    {
        const auto* CurrentItemTarget = Cast<ABaseItem>(BB->GetValueAsObject(TEXT("ItemActor")));
        if (CurrentItemTarget)
        {
            EItemType TargetType = CurrentItemTarget->GetItemType();
            bool bRejectItem = false;

            if (TargetType == EItemType::Pistol && PistolCount >= 1) bRejectItem = true;
            if (TargetType == EItemType::Shotgun && ShotgunCount >= 1) bRejectItem = true;
            if (TargetType == EItemType::Food && FoodCount >= 2) bRejectItem = true;
            if (TargetType == EItemType::Medkit && MedkitCount >= 1) bRejectItem = true;
            if (TargetType == EItemType::Garbage) bRejectItem = true;

            BB->SetValueAsBool(IsInventoryFullKey.SelectedKeyName, bRejectItem);
        }
        else
        {
            BB->SetValueAsBool(IsInventoryFullKey.SelectedKeyName, false);
        }
    }
}
