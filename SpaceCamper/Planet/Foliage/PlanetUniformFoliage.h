// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlanetFoliageBase.h"
#include "PlanetUniformFoliage.generated.h"



UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPACECAMPER_API UPlanetUniformFoliage : public UPlanetFoliageBase
{
	GENERATED_BODY()

public:
        UPlanetUniformFoliage();

        UPROPERTY(EditAnywhere, Category = Foliage)
        int32 NumChunkSamples = 64;
        FRandomStream* CurrentRandom;

protected:
        virtual void CreateFoliageChunk(const FIntPoint& ChunkCoord) override;
};
