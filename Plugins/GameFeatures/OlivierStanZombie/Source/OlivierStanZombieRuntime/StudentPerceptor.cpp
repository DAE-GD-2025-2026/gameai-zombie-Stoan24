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

	auto* Controller = Cast<AAIController>(Cast<APawn>(GetOwner())->GetController());
	if (!Controller) return;
	auto* BB = Controller->GetBlackboardComponent();
	if (!BB) return;


    if (Actor->IsA<ABaseZombie>())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            KnownZombies.AddUnique(Actor);
        }
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

    float ForgetDistance = 1800.f;

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
