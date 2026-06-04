// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/BaseItem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"
#include "StudentPerceptor.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OLIVIERSTANZOMBIERUNTIME_API UStudentPerceptor : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStudentPerceptor();
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	AActor* PopNextVillageHouse();
	bool HasVillageHouses() const { return UnvisitedHouses.Num() > 0; }


	ABaseItem* FindClosestRememberedItem(FVector CurrentLocation, EItemType DesiredType);
	void ForgetPickedUpItem(ABaseItem* ItemActor);

private:

	void UpdateClosestZombie(APawn* Pawn, UBlackboardComponent* BB);

	//Track zombies
	TArray<AActor*> KnownZombies;

	//Track houses
	UPROPERTY()
	TSet<AActor*> GloballyVisitedHouses;

	UPROPERTY()
	TArray<AActor*> UnvisitedHouses;

	UPROPERTY()
	TArray<ABaseItem*> ItemMemory;
};
