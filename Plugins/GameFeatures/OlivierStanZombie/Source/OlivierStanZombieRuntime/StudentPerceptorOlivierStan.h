// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/BaseItem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"
#include "StudentPerceptorOlivierStan.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OLIVIERSTANZOMBIERUNTIME_API UStudentPerceptorOlivierStan : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStudentPerceptorOlivierStan();
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	AActor* PopNextVillageHouse();
	bool HasVillageHouses() const { return UnvisitedHouses.Num() > 0; }


	ABaseItem* FindClosestRememberedItem(FVector CurrentLocation, EItemType DesiredType);
	void ForgetPickedUpItem(ABaseItem* ItemActor);

	TArray<AActor*> GetZombies() { return KnownZombies; }

	UPROPERTY(EditAnywhere, Category = "Flee")
	float DangerRadius{ 400.f };

	UPROPERTY(EditAnywhere, Category = "Flee")
	float RepulsionStrength{ 600.f };

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


	//Looking behind 
	float LookBehindCooldownTimer = 0.0f;
	float LookBehindDurationTimer = 0.0f;
	bool bIsLookingBehind = false;

	const float TimeBetweenGlances = 3.0f;
	const float GlanceDuration = 0.25f;

	bool bIsReactingToDamage = false;
	FRotator DamageTargetRotation;
	float DamageReactionTimer = 0.0f;
	const float DamageReactionDuration = 0.75f;
};
