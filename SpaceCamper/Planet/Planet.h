// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Planet.generated.h"

class UPlanetClusterFoliage;
class UPlanetUniformFoliage;
class UPlanetMeshGenerator;

UCLASS()
class SPACECAMPER_API APlanet : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlanet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadWrite, Category="Editor")
	UPlanetMeshGenerator* EditorMeshGenerator; // 에디터 전용
#endif
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Foliage")
	UPlanetClusterFoliage* GrassFoliage;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Foliage")
	UPlanetClusterFoliage* FlowerFoliage;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Foliage")
	UPlanetClusterFoliage* TreeFoliage;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Foliage")
	UPlanetClusterFoliage* RockFoliage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* PlanetMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* WaterMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UMaterialInterface* PlanetMaterial;
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;
	
	// Editor Events
	UFUNCTION(CallInEditor ,BlueprintImplementableEvent, Category="Planet Events")
	void GeneratePlanetMeshEvent();
	UFUNCTION(CallInEditor ,BlueprintImplementableEvent, Category="Planet Events")
	void BakePlanetMeshEvent();
	
	// Planet Params
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planet Params")
	float PlanetRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planet Params")
	float MountainHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planet Params")
	float OceanHeight;
	
	// Planet Noise Params
	UPROPERTY(EditAnywhere, Category="Planet Params | Noise")
	FVector NoiseFrequencyShift;
	UPROPERTY(EditAnywhere, Category="Planet Params | Noise")
	float NoiseFrequency;
	

	void PlaceProps();
};
