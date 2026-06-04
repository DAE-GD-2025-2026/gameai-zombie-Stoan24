// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_LookupItem_OlivierStan.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "OlivierStanZombieRuntime/StudentPerceptor.h"

UBTT_LookupItem_OlivierStan::UBTT_LookupItem_OlivierStan()
{
	NodeName = TEXT("Find Item In Memory");

	ItemActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_LookupItem_OlivierStan, ItemActorKey), AActor::StaticClass());
	ItemLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_LookupItem_OlivierStan, ItemLocationKey));
}

EBTNodeResult::Type UBTT_LookupItem_OlivierStan::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto* Controller = OwnerComp.GetAIOwner();
	auto* BB = OwnerComp.GetBlackboardComponent();
	if (!Controller || !BB) return EBTNodeResult::Failed;

	auto Pawn = Controller->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	auto* Perceptor = Pawn->FindComponentByClass<UStudentPerceptor>();
	if (!Perceptor) return EBTNodeResult::Failed;


	uint8 EnumValue = BB->GetValueAsEnum(NeededItemTypeKey.SelectedKeyName);
	EItemType DynamicNeededType = static_cast<EItemType>(EnumValue);

	if (DynamicNeededType == EItemType::Garbage)
	{
		return EBTNodeResult::Failed;
	}

	ABaseItem* FoundItem = Perceptor->FindClosestRememberedItem(Pawn->GetActorLocation(), DynamicNeededType);

	if (FoundItem)
	{
		BB->SetValueAsObject(ItemActorKey.SelectedKeyName, FoundItem);
		BB->SetValueAsVector(ItemLocationKey.SelectedKeyName, FoundItem->GetActorLocation());
		BB->SetValueAsBool(TEXT("IsPursuingItem"), true);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
