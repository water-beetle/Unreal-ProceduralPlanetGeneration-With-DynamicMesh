// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "PlanetMeshGenerator.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPACECAMPER_API UPlanetMeshGenerator : public UDynamicMeshComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlanetMeshGenerator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	UStaticMesh* BakeStaticMesh(UStaticMeshDescription* StaticMeshDescription, TArray<UMaterialInterface*> Materials);
	UFUNCTION(BlueprintCallable)
	UDynamicMesh* ApplyPlanetPerlinNoiseToMesh(UDynamicMesh* TargetMesh,
		FGeometryScriptMeshSelection Selection, UGeometryScriptDebug* Debug);
};
