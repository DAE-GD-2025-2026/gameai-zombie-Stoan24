// Fill out your copyright notice in the Description page of Project Settings.


#include "BTS_MonitorStatus_OlivierStan.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Survivor/SurvivorPawn.h"
#include "Common/InventoryComponent.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "Items/BaseItem.h"
#include "OlivierStanZombieRuntime/StudentPerceptorOlivierStan.h"

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



    TArray<ABaseItem*> Items = Inventory->GetInventory();
    bool bInventoryMutated = false;


    //Use Medkit when low health
    if (HealthComp->GetHealth() <= 6)
    {
        for (int i = 0; i < Items.Num(); ++i)
        {
            if (Items[i] && Items[i]->GetItemType() == EItemType::Medkit)
            {
                Inventory->UseItem(i);
                Inventory->RemoveItem(i);
                bInventoryMutated = true;

                GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Low Health --> used Medkit"));
                break;
            }
        }
    }

    //Sync
    if (bInventoryMutated)
    {
        Items = Inventory->GetInventory();
        bInventoryMutated = false;
    }


    //use food when low stamina
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

    //Sync
    if (bInventoryMutated)
    {
        Items = Inventory->GetInventory();
        bInventoryMutated = false;
    }



    //Weapon Cleanup

    //Backward iteration to remove weapons when no more ammo
    for (int i = Items.Num() - 1; i >= 0; --i)
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
            }
        }
    }


    Items = Inventory->GetInventory();

    int PistolCount = 0;
    int ShotgunCount = 0;
    int FoodCount = 0;
    int MedkitCount = 0;
    int TotalItemCount = 0;

    int PistolWithAmmoCount = 0;
    int ShotgunWithAmmoCount = 0;


    //Count all Inventory items
    for (int i = 0; i < Items.Num(); ++i)
    {
        ABaseItem* Item = Items[i];

        if (Item != nullptr)
        {
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
    TotalItemCount += PistolWithAmmoCount + ShotgunWithAmmoCount + FoodCount + MedkitCount;




    //Flag when player has no weapon
    if (PistolWithAmmoCount == 0 && ShotgunWithAmmoCount == 0)
    {
        BB->SetValueAsBool(NeedsWeaponKey.SelectedKeyName, true);
    }
    else
    {
        BB->ClearValue(NeedsWeaponKey.SelectedKeyName);
    }


    //Priority evaluation
    EItemType HighestPriorityNeed = EItemType::Garbage;

    bool bZombieVisible = BB->GetValueAsObject(ZombieActorKey.SelectedKeyName) != nullptr;

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
    else if (ShotgunWithAmmoCount == 0)
    {
        HighestPriorityNeed = EItemType::Shotgun;
    }

    //Fallback shotgun when being chased
    if (bZombieVisible && ShotgunWithAmmoCount == 0 && PistolWithAmmoCount > 0)
    {
        HighestPriorityNeed = EItemType::Shotgun;
    }


    BB->SetValueAsEnum(NeededItemTypeKey.SelectedKeyName, static_cast<uint8>(HighestPriorityNeed));

    Items = Inventory->GetInventory();

    //Filter
    if (TotalItemCount >= Inventory->GetInventoryCapacity())
    {
        BB->SetValueAsBool(IsInventoryFullKey.SelectedKeyName, true);
    }
    else
    {
        BB->SetValueAsBool(IsInventoryFullKey.SelectedKeyName, false);
    }

    //Check Village

    APawn* Pawn = Controller->GetPawn();
    if (!Pawn) return;

    auto* Perceptor = Pawn->FindComponentByClass<UStudentPerceptorOlivierStan>();
    if (Perceptor->HasVillageHouses())
    {
        BB->SetValueAsBool(HasVillageTargetKey.SelectedKeyName, true);
    }
}
