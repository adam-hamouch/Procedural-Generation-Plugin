#include "GenerationWindow.h"
#include "SplineActor.h"
#include "ProceduralPlacementComponent.h"

// Editor
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"

// Slate
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "PropertyCustomizationHelpers.h"
#include "Selection.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Framework/Docking/TabManager.h"

const FName UGenerationWindow::TabName(TEXT("ProceduralGenerationTab"));
TWeakObjectPtr<AActor> UGenerationWindow::TargetActor = nullptr;

void UGenerationWindow::RegisterTabSpawner()
{
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        TabName,
        FOnSpawnTab::CreateStatic(&UGenerationWindow::SpawnTab)
    )
    .SetDisplayName(FText::FromString("Procedural Generation"))
    .SetMenuType(ETabSpawnerMenuType::Hidden);
}

void UGenerationWindow::UnregisterTabSpawner()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
}

void UGenerationWindow::SetTargetActor(AActor* Actor)
{
    TargetActor = Actor;
}

TSharedRef<SDockTab> UGenerationWindow::SpawnTab(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SVerticalBox)

            // Titre
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(10)
            [
                SNew(STextBlock)
                .Text(FText::FromString("Procedural Generation Tool"))
            ]

            // Bouton Spawn Actor
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(10)
            [
                SNew(SButton)
                .Text(FText::FromString("Create Spline Zone"))
                .OnClicked_Static(&UGenerationWindow::OnCreateProceduralActorClicked)
            ]

            // Sélection Actor
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)
            [
                SNew(STextBlock)
                .Text(FText::FromString("Target Procedural Actor"))
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)
            [
                SNew(SButton)
                .Text(FText::FromString("Select Actor in Scene"))
                .OnClicked_Lambda([]()
                {
                    UWorld* World = GEditor->GetEditorWorldContext().World();
                    if (!World) return FReply::Handled();

                    for (TActorIterator<AActor> It(World); It; ++It)
                    {
                        if (It->FindComponentByClass<UProceduralPlacementComponent>())
                        {
                            UGenerationWindow::SetTargetActor(*It);
                            break;
                        }
                    }
                    return FReply::Handled();
                })
            ]

            // Mesh
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)
            [
                SNew(SObjectPropertyEntryBox)
                .AllowedClass(UStaticMesh::StaticClass())
                .ObjectPath_Lambda([]() -> FString
                {
                    if (TargetActor.IsValid())
                    {
                        UProceduralPlacementComponent* Comp = TargetActor->FindComponentByClass<UProceduralPlacementComponent>();
                        if (Comp && Comp->Mesh) return Comp->Mesh->GetPathName();
                    }
                    return FString();
                })
                .OnObjectChanged_Lambda([](const FAssetData& NewAsset)
                {
                    if (TargetActor.IsValid())
                    {
                        UProceduralPlacementComponent* Comp = TargetActor->FindComponentByClass<UProceduralPlacementComponent>();
                        if (Comp) Comp->Mesh = Cast<UStaticMesh>(NewAsset.GetAsset());
                    }
                })
            ]

            // Spline
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)
            [
                SNew(SButton)
                .Text(FText::FromString("Use Selected Spline"))
                .OnClicked_Lambda([]()
                {
                    if (!TargetActor.IsValid()) return FReply::Handled();
                
                    USelection* Selection = GEditor->GetSelectedComponents();
                    for (FSelectedEditableComponentIterator It(*Selection); It; ++It)
                    {
                        if (USplineComponentPG* Spline = Cast<USplineComponentPG>(*It))
                        {
                            if (auto* Comp = TargetActor->FindComponentByClass<UProceduralPlacementComponent>())
                            {
                                Comp->Spline = Spline;
                            }
                            break;
                        }
                    }
                    return FReply::Handled();
                })
            ]
            
            // Spacing
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)
            [
                SNew(SNumericEntryBox<int32>)
                .LabelVAlign(VAlign_Center)
                .Label()[ SNew(STextBlock).Text(FText::FromString("Spacing")) ]
                .Value_Lambda([]() -> TOptional<int32>
                {
                    if (TargetActor.IsValid())
                    {
                        UProceduralPlacementComponent* Comp = TargetActor->FindComponentByClass<UProceduralPlacementComponent>();
                        if (Comp) return Comp->Spacing;
                    }
                    return 0;
                })
                .OnValueChanged_Lambda([](int32 NewValue)
                {
                    if (TargetActor.IsValid())
                    {
                        UProceduralPlacementComponent* Comp = TargetActor->FindComponentByClass<UProceduralPlacementComponent>();
                        if (Comp) Comp->Spacing = NewValue;
                    }
                })
            ]

            // Seed
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)
            [
                SNew(SNumericEntryBox<float>)
                .LabelVAlign(VAlign_Center)
                .Label()[ SNew(STextBlock).Text(FText::FromString("Seed")) ]
                .Value_Lambda([]() -> TOptional<float>
                {
                    if (TargetActor.IsValid())
                    {
                        UProceduralPlacementComponent* Comp = TargetActor->FindComponentByClass<UProceduralPlacementComponent>();
                        if (Comp) return Comp->Seed;
                    }
                    return 1.f;
                })
                .OnValueChanged_Lambda([](float NewValue)
                {
                    if (TargetActor.IsValid())
                    {
                        UProceduralPlacementComponent* Comp = TargetActor->FindComponentByClass<UProceduralPlacementComponent>();
                        if (Comp) Comp->Seed = NewValue;
                    }
                })
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(10)
            [
                SNew(SButton)
                .Text(FText::FromString("Generate"))
                .OnClicked_Lambda([]() -> FReply
                {
                    if (TargetActor.IsValid())
                    {
                        UProceduralPlacementComponent* Comp = TargetActor->FindComponentByClass<UProceduralPlacementComponent>();
                        if (Comp)
                        {
                            Comp->Generate(); 
                        }
                    }
                    return FReply::Handled();
                })
            ]

            // Clear Meshes
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)
            [
                SNew(SButton)
                .Text(FText::FromString("Clear"))
                .OnClicked_Lambda([]() -> FReply
                {
                    if (TargetActor.IsValid())
                    {
                        UProceduralPlacementComponent* Comp = TargetActor->FindComponentByClass<UProceduralPlacementComponent>();
                        if (Comp && Comp->Spline->ISMComp)
                        {
                            Comp->Spline->ISMComp->ClearInstances(); 
                        }
                    }
                    return FReply::Handled();
                })
            ]
        ];
}

FReply UGenerationWindow::OnCreateProceduralActorClicked()
{
    if (!GEditor) return FReply::Handled();

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return FReply::Handled();

    const FVector Location(0.f, 0.f, 100.f);
    const FRotator Rotation = FRotator::ZeroRotator;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ASplineActor* NewActor = World->SpawnActor<ASplineActor>(
        ASplineActor::StaticClass(),
        Location,
        Rotation,
        Params
    );

    if (NewActor)
    {
        UGenerationWindow::SetTargetActor(NewActor);
    }

    return FReply::Handled();
}