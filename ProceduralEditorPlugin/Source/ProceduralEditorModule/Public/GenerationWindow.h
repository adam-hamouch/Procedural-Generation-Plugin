#pragma once

#include "CoreMinimal.h"
#include "Widgets/Docking/SDockTab.h"
#include "UObject/WeakObjectPtrTemplates.h"

class ASplineActor;

class UGenerationWindow
{
public:
	static void RegisterTabSpawner();
	static void UnregisterTabSpawner();
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);

	static FReply OnCreateProceduralActorClicked();
	static void SetTargetActor(AActor* Actor);

	static TWeakObjectPtr<AActor> TargetActor;

	// Déclare juste ici
	static const FName TabName;
};