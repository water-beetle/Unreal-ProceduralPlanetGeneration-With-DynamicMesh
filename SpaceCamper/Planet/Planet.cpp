// Fill out your copyright notice in the Description page of Project Settings.


#include "Planet.h"

#include "Utility/Generators/PlanetGenerator.h"


// Sets default values
APlanet::APlanet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlanetMesh = CreateDefaultSubobject<UStaticMeshComponent>("PlanetMesh");
	RootComponent = PlanetMesh;
	
	WaterMesh = CreateDefaultSubobject<UStaticMeshComponent>("WaterMesh");
	WaterMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APlanet::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlanet::PlaceProps()
{
}


