// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Items/ItemType.h"
#include "BTT_LookupItem_OlivierStan.generated.h"

enum class EItemType : uint8;
/**
 * 
 */
UCLASS()
class OLIVIERSTANZOMBIERUNTIME_API UBTT_LookupItem_OlivierStan : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_LookupItem_OlivierStan();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:

	UPROPERTY(EditAnywhere, Category = "Config")
	FBlackboardKeySelector NeededItemTypeKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ItemActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ItemLocationKey;
};
