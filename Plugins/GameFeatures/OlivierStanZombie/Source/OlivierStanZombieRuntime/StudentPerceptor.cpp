// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptor.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Zombies/BaseZombie.h"

#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Village/House/House.h"

UStudentPerceptor::UStudentPerceptor()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStudentPerceptor::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptor::OnPerceptionUpdated);
	}
}

void UStudentPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

    auto* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn) return;
	auto* Controller = Cast<AAIController>(Cast<APawn>(GetOwner())->GetController());
	if (!Controller) return;
	auto* BB = Controller->GetBlackboardComponent();
	if (!BB) return;


    if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
    {
        FVector LookAtVector = Actor->GetActorLocation() - Pawn->GetActorLocation();
        LookAtVector.Z = 0.0f;

        if (!LookAtVector.IsNearlyZero())
        {
            DamageTargetRotation = LookAtVector.Rotation();
            bIsReactingToDamage = true;
            DamageReactionTimer = 0.0f;


            if (Actor->IsA<ABaseZombie>())
            {
                KnownZombies.AddUnique(Actor);
            }
        }
        return;
    }


	if (!Stimulus.WasSuccessfullySensed())return;

    if (Actor->IsA<ABaseZombie>())
    {
        KnownZombies.AddUnique(Actor);
    }


	if (Actor->IsA<ABaseItem>())
    {
        ABaseItem* SensedItem = Cast<ABaseItem>(Actor);
        if (SensedItem)
        {
            ItemMemory.AddUnique(SensedItem);

            bool bAlreadyPursuing = BB->GetValueAsBool(TEXT("IsPursuingItem"));
            if (!bAlreadyPursuing)
            {
                BB->SetValueAsVector(TEXT("ItemLocation"), SensedItem->GetActorLocation());
                BB->SetValueAsObject(TEXT("ItemActor"), SensedItem);
            }
        }
    }

    if (Actor->IsA<AHouse>())
    {
        if (UnvisitedHouses.Num() == 0 && !GloballyVisitedHouses.Contains(Actor))
        {
            UnvisitedHouses.AddUnique(Actor);

            if (UnvisitedHouses.Num() > 0)
            {
                BB->SetValueAsBool(TEXT("HasVillageTarget"), true);
            }
        }
    }
}

void UStudentPerceptor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    auto* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn) return;

    auto* Controller = Cast<AAIController>(Pawn->GetController());
    if (!Controller) return;

    auto* BB = Controller->GetBlackboardComponent();
    if (!BB) return;

    UpdateClosestZombie(Pawn, BB);
}

void UStudentPerceptor::UpdateClosestZombie(APawn* Pawn, UBlackboardComponent* BB)
{
    KnownZombies.RemoveAll([](AActor* A) { return !IsValid(A); });

    AActor* Closest = nullptr;
    float ClosestDist = FLT_MAX;

    const float ForgetDistance = 1800.f;

    TArray<AActor*> ZombiesToForget;

    for (AActor* Zombie : KnownZombies)
    {
        float Dist = FVector::Dist(Pawn->GetActorLocation(), Zombie->GetActorLocation());

        if (Dist > ForgetDistance)
        {
            ZombiesToForget.Add(Zombie);
            continue;
        }

        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            Closest = Zombie;
        }
    }

    //Remove zombies that are out of range
    for (AActor* StaleZombie : ZombiesToForget)
    {
        KnownZombies.Remove(StaleZombie);
    }

    if (Closest)
    {
        BB->SetValueAsVector(TEXT("ZombieLocation"), Closest->GetActorLocation());
        BB->SetValueAsObject(TEXT("ZombieActor"), Closest);
    }
    else
    {
        BB->ClearValue(TEXT("ZombieActor"));
    }
}

AActor* UStudentPerceptor::PopNextVillageHouse()
{
    //Remove old houses
    if (UnvisitedHouses.Num() == 0) return nullptr;

    AActor* NextHouse = UnvisitedHouses[0];
    UnvisitedHouses.RemoveAt(0);

    if (NextHouse)
    {
        GloballyVisitedHouses.Add(NextHouse);
    }
    return NextHouse;
}

ABaseItem* UStudentPerceptor::FindClosestRememberedItem(FVector CurrentLocation, EItemType DesiredType)
{
    ABaseItem* BestItem = nullptr;
    float BestDist = FLT_MAX;


    //Get location of item closest to the player
    for (ABaseItem* Item : ItemMemory)
    {
        if (Item && Item->GetItemType() == DesiredType)
        {
            float Dist = FVector::Dist(CurrentLocation, Item->GetActorLocation());
            if (Dist < BestDist)
            {
                BestDist = Dist;
                BestItem = Item;
            }
        }
    }
    return BestItem;
}

void UStudentPerceptor::ForgetPickedUpItem(ABaseItem* ItemActor)
{
    if (ItemActor)
    {
        ItemMemory.Remove(ItemActor);
    }
}