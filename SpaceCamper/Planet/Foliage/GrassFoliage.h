// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlanetFoliageBase.h"
#include "GrassFoliage.generated.h"



UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPACECAMPER_API UGrassFoliage : public UPlanetFoliageBase
{
	GENERATED_BODY()

public:
        UGrassFoliage();

protected:
        virtual void CreateFoliageChunk(const FIntPoint& ChunkCoord) override;
};
