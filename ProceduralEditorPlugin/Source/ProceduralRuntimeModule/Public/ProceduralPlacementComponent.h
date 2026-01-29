// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Landscape.h"
#include "SplineComponentPG.h"
#include "SceneInterface.h"
#include "LandscapeComponent.h"
#include "EngineUtils.h"
#include "ProceduralPlacementComponent.generated.h"

USTRUCT()
struct FGridCell
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<float> PointsIndex;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROCEDURALRUNTIMEMODULE_API UProceduralPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProceduralPlacementComponent();

	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
	TObjectPtr<UStaticMesh> Mesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
    TObjectPtr<USplineComponentPG> Spline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
	int32 Seed = 123;

	UPROPERTY()
	TArray<FVector> Positions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
	float Spacing = 0.0f;

	UPROPERTY()
	TArray<FVector> SplinePoints;
	
	float GridX = 0.0f;
	float GridY = 0.0f;
	
	UFUNCTION(BlueprintCallable, Category="Placement")
	void Generate();

	UFUNCTION(BlueprintCallable, Category="Placement")
	void GenerateFunction();

	UFUNCTION()
	void ApplyPoints(const TArray<FVector>& Points);
	
	// Points generation algorithm
	UFUNCTION()
	void PoissonDiskAlgo(TArray<FVector>& Points);

	UFUNCTION()
	void ProjectPoints();

	UFUNCTION()
	bool IsValid(FVector Candidate, TArray<FVector>& Points, FVector Max, FVector Min, float CellSIze, TArray<FGridCell> Grids);

	UFUNCTION()
	bool IsInside(FVector Candidate);

	UFUNCTION()
	FVector GetMinPoint();

	UFUNCTION()
	FVector GetMaxPoint();

	UFUNCTION()
	void CacheSpline();
};
