// Fill out your copyright notice in the Description page of Project Settings.


#include "Planet.h"

#include "Foliage/FlowerFoliage.h"
#include "Foliage/GrassFoliage.h"
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
	FlowerFoliage = CreateDefaultSubobject<UFlowerFoliage>("FlowerFoliage");
	
	// Init Planet Noise Params
	RandomSeed = 1999;
	Random.Initialize(RandomSeed);
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
			// 예시로 빨간색 적용
			DynamicMaterial->SetScalarParameterValue(FName("MountinHeigh"), MountainHeight);
			DynamicMaterial->SetScalarParameterValue(FName("OceanHeight"), OceanHeight);

			// 메시 컴포넌트에 새 머티리얼 적용
			PlanetMesh->SetMaterial(0, DynamicMaterial);
		}
	}
	
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


