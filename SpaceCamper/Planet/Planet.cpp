// Fill out your copyright notice in the Description page of Project Settings.


#include "Planet.h"

#include "GrassFoliage.h"
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

	GrassFoliage = CreateDefaultSubobject<UGrassFoliage>("GrassFoliage");
	
	// Init Planet Noise Params
	RandomSeed = 1999;
	Random.Initialize(RandomSeed);
}

// Called when the game starts or when spawned
void APlanet::BeginPlay()
{
	Super::BeginPlay();
	
}

#if WITH_EDITOR
void APlanet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr)
		? PropertyChangedEvent.Property->GetFName()
		: NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(APlanet, RandomSeed))
	{
		// RandomSeed가 바뀌면 Random 업데이트
		Random.Initialize(RandomSeed);

		UE_LOG(LogTemp, Log, TEXT("RandomSeed updated: %d"), RandomSeed);
	}
}
#endif

// Called every frame
void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlanet::PlaceProps()
{
}


