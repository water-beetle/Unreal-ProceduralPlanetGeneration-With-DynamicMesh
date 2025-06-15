// Fill out your copyright notice in the Description page of Project Settings.


#include "Planet.h"

#include "Foliage/PlanetClusterFoliage.h"
#include "Foliage/PlanetUniformFoliage.h"
#include "Utility/Generators/PlanetMeshGenerator.h"


// Sets default values
APlanet::APlanet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlanetMesh = CreateDefaultSubobject<UStaticMeshComponent>("PlanetMesh");
	RootComponent = PlanetMesh;
	
	WaterMesh = CreateDefaultSubobject<UStaticMeshComponent>("WaterMesh");
	WaterMesh->SetupAttachment(RootComponent);

	EditorMeshGenerator = CreateDefaultSubobject<UPlanetMeshGenerator>("PlanetMeshGenerator");
	EditorMeshGenerator->SetupAttachment(RootComponent);

	GrassFoliage = CreateDefaultSubobject<UPlanetClusterFoliage>("GrassFoliage");
	FlowerFoliage = CreateDefaultSubobject<UPlanetClusterFoliage>("FlowerFoliage");
	TreeFoliage = CreateDefaultSubobject<UPlanetClusterFoliage>("TreeFoliage");
	RockFoliage = CreateDefaultSubobject<UPlanetClusterFoliage>("RockFoliage");
}

// Called when the game starts or when spawned
void APlanet::BeginPlay()
{
	Super::BeginPlay();

	if (PlanetMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(PlanetMaterial, this);
		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValue(FName("MountinHeigh"), MountainHeight);
			DynamicMaterial->SetScalarParameterValue(FName("OceanHeight"), OceanHeight);

			PlanetMesh->SetMaterial(0, DynamicMaterial);
		}
	}
	
}

// Called every frame
void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlanet::PlaceProps()
{
}


