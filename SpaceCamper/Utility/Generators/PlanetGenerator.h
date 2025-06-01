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

public:

	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* TreeFoliageComponent;
	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* GrassFoliageComponent;
	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* RockFoliageComponent;
	UPROPERTY(EditAnywhere, Category = "Foliage")
	UStaticMesh* TreeFoliageMesh;
	UPROPERTY(EditAnywhere, Category = "Foliage")
	UStaticMesh* GrassFoliageMesh;
	UPROPERTY(EditAnywhere, Category = "Foliage")
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
	
	void GenerateTreeFoliage(const FVector& Pos, const FVector& Normal, float MinHeightSquared, float MaxHeightSquared) const;
	void GenerateGrassFoliage(const FVector& Pos, const FVector& Normal, float MinHeightSquared, float MaxHeightSquared) const;
	void GenerateRockFoliage(const FVector& Pos, const FVector& Normal, float MinHeightSquared, float MaxHeightSquared) const;

	FVector GetNoisePosition(const FVector& Pos);
	
	bool CheckHeigntInRange(float minHeight, float maxHeight, const FVector& Pos) const;

};
