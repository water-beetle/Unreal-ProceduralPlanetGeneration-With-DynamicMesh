// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlanetFoliageBase.h"
#include "FlowerFoliage.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPACECAMPER_API UFlowerFoliage : public UPlanetFoliageBase
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFlowerFoliage();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
protected:
	virtual void CreateFoliageChunk(const FIntPoint& ChunkCoord) override;
};
