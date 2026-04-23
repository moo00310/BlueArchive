// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BAResourceSubsystem.h"

void UBAResourceSubsystem::InitializeFromServer(const TArray<FBAResourceEntry>& InResources, const FString& InUserName, int32 InUserLevel)
{
	Resources.Empty();
	for (const FBAResourceEntry& Entry : InResources)
		Resources.Add(Entry.ResourceType, Entry.Amount);

	UserName = InUserName;
	UserLevel = InUserLevel;

	for (const auto& Pair : Resources)
		OnResourceChanged.Broadcast(Pair.Key, Pair.Value);

	OnUserLevelChanged.Broadcast(UserLevel);
	OnUserNameChanged.Broadcast(UserName);
}

int32 UBAResourceSubsystem::GetResource(EResourceType ResourceType) const
{
	const int32* Value = Resources.Find(ResourceType);
	return Value ? *Value : 0;
}

void UBAResourceSubsystem::AddResource(EResourceType ResourceType, int32 Amount)
{
	if (Amount <= 0) return;
	int32& Current = Resources.FindOrAdd(ResourceType);
	Current += Amount;
	OnResourceChanged.Broadcast(ResourceType, Current);
}

bool UBAResourceSubsystem::SpendResource(EResourceType ResourceType, int32 Amount)
{
	if (Amount <= 0) return true;
	int32* Current = Resources.Find(ResourceType);
	if (!Current || *Current < Amount) return false;
	*Current -= Amount;
	OnResourceChanged.Broadcast(ResourceType, *Current);
	return true;
}

void UBAResourceSubsystem::SetResource(EResourceType ResourceType, int32 Value)
{
	Resources.FindOrAdd(ResourceType) = Value;
	OnResourceChanged.Broadcast(ResourceType, Value);
}

void UBAResourceSubsystem::SetUserLevel(int32 Level)
{
	if (Level < 1) return;
	UserLevel = Level;
	OnUserLevelChanged.Broadcast(UserLevel);
}

void UBAResourceSubsystem::SetUserName(const FString& Name)
{
	if (Name.IsEmpty()) return;
	UserName = Name;
	OnUserNameChanged.Broadcast(UserName);
}
