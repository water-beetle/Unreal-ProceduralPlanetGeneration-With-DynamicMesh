// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrassFoliage.generated.h"


class UHierarchicalInstancedStaticMeshComponent;
class APlanet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPACECAMPER_API UGrassFoliage : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGrassFoliage();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Ocahedral 매핑 함수
    static FVector2D OctahedralEncode(const FVector& N); // Octahedral 맵핑: 구면 좌표계 -> 2D 정사각형
    static FVector  OctahedralDecode(const FVector2D& UV); // 2D 정사각형 -> 구면 좌표계
    static FIntPoint GetChunkCoordFromOctahedral(const FVector& N, int32 NumChunks); // 2D 좌표를 Chunk 인덱스로 변환

	// Chunk 처리 함수
	void UpdateGrassChunks(const FIntPoint& CenterChunk);
	void CreateGrassChunk(const FIntPoint& ChunkCoord);

private:
	FIntPoint LastCameraChunk = FIntPoint(-1, -1);

	// 파라미터
	int32 NumChunks = 32;                // Octahedral 2D 분할 갯수
	int32 NumChunkSamples = 64;

	float CurrentRadius;
	float CurrentNoiseFrequency;
	FVector CurrentNoiseShift;
	FRandomStream* CurrentRandom;
	
	TMap<FIntPoint, UHierarchicalInstancedStaticMeshComponent*> GrassChunkMap;

	UPROPERTY(EditAnywhere)
	UStaticMesh* GrassFoliageMesh;
};
