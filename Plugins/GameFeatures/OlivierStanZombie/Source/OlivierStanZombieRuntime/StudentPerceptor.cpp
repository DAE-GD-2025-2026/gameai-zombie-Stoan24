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
    }

    if (Actor->IsA<AHouse>() && Stimulus.WasSuccessfullySensed())
    {
        if (UnvisitedHouses.Num() == 0 && !GloballyVisitedHouses.Contains(Actor))
        {
            FVector SearchOrigin = Actor->GetActorLocation();
            TArray<FOverlapResult> Overlaps;
            FCollisionShape Sphere = FCollisionShape::MakeSphere(3500.f); //Cheat? Find houses surrounding the 'seen' house
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(Pawn);

            if (GetWorld()->OverlapMultiByObjectType(Overlaps, SearchOrigin, FQuat::Identity, FCollisionObjectQueryParams::AllObjects, Sphere, Params))
            {
                for (const FOverlapResult& Result : Overlaps)
                {
                    AActor* OverlappedActor = Result.GetActor();
                    if (OverlappedActor && OverlappedActor->IsA<AHouse>())
                    {
                        if (!GloballyVisitedHouses.Contains(OverlappedActor))
                        {
                            UnvisitedHouses.AddUnique(OverlappedActor);
                        }
                    }
                }
            }

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
    UpdateMovemennt(DeltaTime, Pawn, Controller);

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

void UStudentPerceptor::UpdateMovemennt(float DeltaTime, APawn* Pawn, const AAIController* Controller)
{
    if (Controller->GetMoveStatus() == EPathFollowingStatus::Type::Moving)
    {
        auto* MovementComp = Pawn->FindComponentByClass<UCharacterMovementComponent>();
        if (MovementComp)
        {
            FVector CharacterLocation = Pawn->GetActorLocation();


            FVector BasePathDirection = Controller->GetImmediateMoveDestination() - CharacterLocation;
            BasePathDirection.Z = 0.0f;
            BasePathDirection.Normalize();

            FVector TargetVelocity = BasePathDirection * MovementComp->MaxWalkSpeed;
            FVector TotalRepulsionForce = FVector::ZeroVector;


            for (AActor* Zombie : KnownZombies)
            {
                if (IsValid(Zombie))
                {
                    FVector ZombieLocation = Zombie->GetActorLocation();
                    float Distance = FVector::Dist2D(CharacterLocation, ZombieLocation);


                    if (Distance < DangerRadius && Distance > 1.0f)
                    {
                        FVector PushDirection = CharacterLocation - ZombieLocation;
                        PushDirection.Z = 0.0f;
                        PushDirection.Normalize();


                        float ProximityFactor = 1.0f - (Distance / DangerRadius);
                        TotalRepulsionForce += PushDirection * (RepulsionStrength * ProximityFactor);
                    }
                }
            }


            if (!TotalRepulsionForce.IsZero())
            {
                TargetVelocity += TotalRepulsionForce;
                TargetVelocity = TargetVelocity.GetClampedToMaxSize(MovementComp->MaxWalkSpeed);

                MovementComp->Velocity = FMath::VInterpTo(MovementComp->Velocity, TargetVelocity, DeltaTime, 8.0f);


                if (MovementComp->Velocity.SizeSquared() > 100.0f)
                {
                    FRotator TargetRotation = MovementComp->Velocity.Rotation();
                    TargetRotation.Pitch = 0.0f;
                    TargetRotation.Roll = 0.0f;
                    Pawn->SetActorRotation(FMath::RInterpTo(Pawn->GetActorRotation(), TargetRotation, DeltaTime, 10.0f));
                }
            }
        }
    }
}

AActor* UStudentPerceptor::PopNextVillageHouse()
{
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