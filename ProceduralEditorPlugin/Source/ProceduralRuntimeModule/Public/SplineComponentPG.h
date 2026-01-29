// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "SplineComponentPG.generated.h"

class UInstancedStaticMeshComponent;
/**
 * 
 */
UCLASS(ClassGroup=(Procedural), meta=(BlueprintSpawnableComponent))
class PROCEDURALRUNTIMEMODULE_API USplineComponentPG : public USplineComponent
{
	GENERATED_BODY()

public :
	
	UPROPERTY(VisibleAnywhere, Category = "Procedural")
	UInstancedStaticMeshComponent* ISMComp;

};
