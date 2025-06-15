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
        /** 최소 클러스터 개수 */
        UPROPERTY(EditAnywhere, Category = Foliage)
        int32 MinClusters = 2;

        /** 최대 클러스터 개수 */
        UPROPERTY(EditAnywhere, Category = Foliage)
        int32 MaxClusters = 4;

        /** 한 클러스터 당 인스턴스 수 */
        UPROPERTY(EditAnywhere, Category = Foliage)
        int32 ClusterSize = 10;

        /** 클러스터 확산 정도 (청크 단위 비율) */
        UPROPERTY(EditAnywhere, Category = Foliage)
        float ClusterSpread = 0.1f;

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
