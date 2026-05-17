// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptor.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Zombies/BaseZombie.h"
#include "Items/BaseItem.h"

#include "Engine/Engine.h"

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


    if (Actor->IsA<ABaseZombie>())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            KnownZombies.AddUnique(Actor);

            BB->SetValueAsVector(TEXT("ZombieLocation"), Actor->GetActorLocation());
            BB->SetValueAsObject(TEXT("ZombieActor"), Actor);
        }
        else
        {
            KnownZombies.Remove(Actor);
        }

        UpdateClosestZombie(Pawn, BB);
    }
    else if (Actor->IsA<ABaseItem>())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            bool bAlreadyPursuing = BB->GetValueAsBool(TEXT("IsPursuingItem"));

            if (!bAlreadyPursuing)
            {
                auto* Item = Cast<ABaseItem>(Actor);
                if (Item)
                {
                    BB->SetValueAsVector(TEXT("ItemLocation"), Item->GetActorLocation());
                    BB->SetValueAsObject(TEXT("ItemActor"), Item);
                }
            }
        }
    }
}

void UStudentPerceptor::UpdateClosestZombie(APawn* Pawn, UBlackboardComponent* BB)
{
    KnownZombies.RemoveAll([](AActor* A) { return !IsValid(A); });

    //Find closest
    AActor* Closest = nullptr;
    float ClosestDist = FLT_MAX;

    for (AActor* Zombie : KnownZombies)
    {
        float Dist = FVector::Dist(Pawn->GetActorLocation(), Zombie->GetActorLocation());
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            Closest = Zombie;
        }
    }

    if (Closest)
    {
        BB->SetValueAsVector(TEXT("ZombieLocation"), Closest->GetActorLocation());
        BB->SetValueAsObject(TEXT("ZombieActor"), Closest);
    }
}
