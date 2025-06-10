// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "GeometryScript/MeshDecompositionFunctions.h"
#include "Components/ActorComponent.h"
#include "DynamicMesh/MeshNormals.h"
#include "PlanetGenerator.generated.h"

class UHierarchicalInstancedStaticMeshComponent;

UCLASS()
class SPACECAMPER_API APlanetGenerator : public ADynamicMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	APlanetGenerator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostRegisterAllComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	
public:

	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* TreeFoliageComponent;
	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* GrassFoliageComponent;
	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* RockFoliageComponent;
	UPROPERTY(EditAnywhere, Category = "Planet Foliage Params")
	UStaticMesh* TreeFoliageMesh;
	UPROPERTY(EditAnywhere, Category = "Planet Foliage Params")
	UStaticMesh* GrassFoliageMesh;
	UPROPERTY(EditAnywhere, Category = "Planet Foliage Params")
	UStaticMesh* RockFoliageMesh;

	// Planet Generation Params
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planet Params")
	int PlanetRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planet Params")
	float MountainHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planet Params")
	float OceanHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planet Params | Noise")
	float NoiseFrequency;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planet Params | Noise")
	FVector NoiseFrequencyShift;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planet Params | Noise")
	int RandomSeed;

	UFUNCTION(CallInEditor ,BlueprintImplementableEvent, Category="Planet Events")
	void GeneratePlanetMeshEvent();
	UFUNCTION(CallInEditor, BlueprintImplementableEvent, Category="Planet Events")
	void GenerateFoliageMeshEvent();
	UFUNCTION(CallInEditor, BlueprintImplementableEvent, Category="Planet Events")
	void BakePlanetMeshEvent();

	UPROPERTY(EditAnywhere, Category="Planet Foliage Params")
	float TreeDensity = 0.2f;
	UPROPERTY(EditAnywhere, Category="Planet Foliage Params")
	float GrassDensity = 0.2f;
	UPROPERTY(EditAnywhere, Category="Planet Foliage Params")
	float RockDensity = 0.2f;
	
	UFUNCTION(BlueprintCallable)
	void UpdateNormals(UDynamicMesh* DynamicMesh);
	UFUNCTION(BlueprintCallable)
	UStaticMesh* BakeStaticMesh(UStaticMeshDescription* StaticMeshDescription, TArray<UMaterialInterface*> Materials, int ind);
	UFUNCTION(BlueprintCallable)
	UDynamicMesh* ApplyPlanetPerlinNoiseToMesh(UDynamicMesh* TargetMesh,
		FGeometryScriptMeshSelection Selection, UGeometryScriptDebug* Debug);
	UFUNCTION(BlueprintCallable)
	void GenerateFoliage(UDynamicMesh* DynamicMesh);

private:
	UE::Geometry::FMeshNormals PlanetNormals = nullptr;

	// Chunk 관리
	UPROPERTY()
	TMap<FIntPoint, UHierarchicalInstancedStaticMeshComponent*> GrassChunkMap;
	// 구형 Chunk 세팅
	const float LatitudeStep = 10.f;  // 위도 10도 간격
	const float LongitudeStep = 10.f; // 경도 10도 간격
	const int32 TotalLonSteps = 360 / 10;
	FIntPoint LastCameraChunk = FIntPoint::ZeroValue;
	
	void UpdateGrassChunks(const FIntPoint& CenterChunk);
	void CreateGrassChunk(const FIntPoint& ChunkCoord);
	
	void GenerateTreeFoliage(const FVector& Pos, const FVector& Normal, float MinHeightSquared, float MaxHeightSquared) const;
	void GenerateGrassFoliage(float MinHeightSquared, float MaxHeightSquared) const;
	void GenerateRockFoliage(const FVector& Pos, const FVector& Normal, float MinHeightSquared, float MaxHeightSquared) const;

	FVector GetNoisePosition(const FVector& Pos, const FVector3d Offsets[3]) const;
	
	bool CheckHeightInRange(float minHeight, float maxHeight, const FVector& Pos) const;

};
