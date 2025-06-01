// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetGenerator.h"
#include "DynamicMeshEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "DynamicMesh/MeshNormals.h"


// Sets default values for this component's properties
APlanetGenerator::APlanetGenerator()
{
	TreeFoliageComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("TreeFoliageComponent"));
	GrassFoliageComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("GrassFoliageComponent"));
	RockFoliageComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("RockFoliageComponent"));
	TreeFoliageComponent->SetupAttachment(RootComponent);
	GrassFoliageComponent->SetupAttachment(RootComponent);
	RockFoliageComponent->SetupAttachment(RootComponent);
}


// Called when the game starts
void APlanetGenerator::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void APlanetGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (GetDynamicMeshComponent())
	{
		GetDynamicMeshComponent()->UpdateBounds();
		GetDynamicMeshComponent()->MarkRenderStateDirty();
	}
}

void APlanetGenerator::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	if (GetDynamicMeshComponent())
	{
		GetDynamicMeshComponent()->UpdateBounds();
		GetDynamicMeshComponent()->MarkRenderStateDirty();
	}
}

void APlanetGenerator::UpdateNormals(UDynamicMesh* DynamicMesh)
{
	DynamicMesh->EditMesh([&](FDynamicMesh3& Mesh)
	{
		// 1. Attribute 시스템 없으면 생성
		if (!Mesh.HasAttributes())
		{
			Mesh.EnableAttributes();  // Adds FDynamicMeshAttributeSet
		}

		// 2. VertexNormals 속성이 없으면 생성
		if (!Mesh.HasVertexNormals())
		{
			Mesh.EnableVertexNormals(FVector3f::UnitY()); // 기본 노멀 방향
		}
	});
}

UStaticMesh* APlanetGenerator::BakeStaticMesh(UStaticMeshDescription* StaticMeshDescription, TArray<UMaterialInterface*> Materials, int ind)
{
	FString Path = FString(TEXT("/Game/Planet/Meshes/GeneratedPlanets/"));
	FString ActorName = GetName();
	FString MeshName = FString::Printf(TEXT("SM_Planet_%d"), ind);
	FString PackageName = Path + MeshName;

	UPackage* Package = CreatePackage(*PackageName);
	UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, FName(*MeshName), RF_Public | RF_Standalone);

	StaticMesh->bAllowCPUAccess = true;
	StaticMesh->NeverStream = true;
	StaticMesh->InitResources();
	StaticMesh->SetLightingGuid();

	for (int i = 0; i < Materials.Num(); ++i)
	{
		StaticMesh->GetStaticMaterials().Add(FStaticMaterial(Materials[i]));		
	}

#if WITH_EDITOR
	StaticMesh->PostEditChange();
#endif

	StaticMesh->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(StaticMesh);

	return StaticMesh;
}

UDynamicMesh* APlanetGenerator::ApplyPlanetPerlinNoiseToMesh(UDynamicMesh* TargetMesh,
	FGeometryScriptMeshSelection Selection, UGeometryScriptDebug* Debug)
{
	if (TargetMesh == nullptr)
	{
		return TargetMesh;
	}


	TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh) 
	{
		FRandomStream Random(RandomSeed);
		FVector3d Offsets[3];
		for (int k = 0; k < 3; ++k)
		{
			const float RandomOffset = 10000.0f * Random.GetFraction();
			Offsets[k] = FVector3d(RandomOffset, RandomOffset, RandomOffset);
			Offsets[k] += (FVector3d)NoiseFrequencyShift;
		}
		
		UE::Geometry::FMeshNormals Normals(&EditMesh);
		auto GetDisplacedPosition = [&EditMesh, &Offsets, &Normals, this](int32 VertexID)
		{
			FVector3d Pos = EditMesh.GetVertex(VertexID);
			float Magnitude = PlanetRadius * 0.1f;

			FVector3d Displacement;
			for (int32 k = 0; k < 3; ++k)
			{
				FVector NoisePos = (FVector)((double)NoiseFrequency * (Pos + Offsets[k]));
				Displacement[k] = Magnitude * FMath::PerlinNoise3D(NoiseFrequency * NoisePos);
			}
			Pos += Displacement;
			
			return Pos;
		};

		if (Selection.IsEmpty())
		{ 
			ParallelFor(EditMesh.MaxVertexID(), [&](int32 VertexID)
			{
				if (EditMesh.IsVertex(VertexID))
				{
					EditMesh.SetVertex(VertexID, GetDisplacedPosition(VertexID));
				}
			});
		}
		else
		{
			Selection.ProcessByVertexID(EditMesh, [&](int32 VertexID)
			{
				EditMesh.SetVertex(VertexID, GetDisplacedPosition(VertexID));
			});
		}

	}, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);

	return TargetMesh;
}

void APlanetGenerator::GenerateFoliage(UDynamicMesh* DynamicMesh)
{	
	TreeFoliageComponent->ClearInstances();
	GrassFoliageComponent->ClearInstances();

	if(TreeFoliageMesh)
		TreeFoliageComponent->SetStaticMesh(TreeFoliageMesh);
	if(GrassFoliageMesh)
		GrassFoliageComponent->SetStaticMesh(GrassFoliageMesh);
	
	DynamicMesh->ProcessMesh([&](const FDynamicMesh3& Mesh)
	{
		UE::Geometry::FMeshNormals Normals(&Mesh);
		Normals.ComputeVertexNormals();

		PlanetNormals = Normals;
		
		float OceanHeightSquared = OceanHeight * OceanHeight;
		float MountainHeightSquared = MountainHeight * MountainHeight;
		
		for (int32 VertexID : Mesh.VertexIndicesItr())
		{
			FVector3d Pos = Mesh.GetVertex(VertexID);
			FVector3d Normal = Normals[VertexID];
			
			if(TreeFoliageMesh)
			{
				// 나무 위치를 Normal Vector에 수직으로 하니 어색함 -> 그냥 위치로 하는게 더 보기좋음
				GenerateTreeFoliage(Pos, Normal, OceanHeightSquared, MountainHeightSquared);
				GenerateGrassFoliage(Pos, Normal, OceanHeightSquared, MountainHeightSquared);
			}
		}
	});
}

void APlanetGenerator::GenerateTreeFoliage(const FVector& Pos, const FVector& Normal, float MinHeightSquared, float MaxHeightSquared) const
{
	if (FMath::FRand() < TreeDensity)
	{
		if(CheckHeigntInRange(MinHeightSquared + 30, MaxHeightSquared, Pos)) // 가끔씩 바다랑 겹치는 경우가 있어서 +30 더해줌
		{
			FRotator FoliageRot = FRotationMatrix::MakeFromZ(Pos).Rotator();
			FVector Offset = Normal * -50.0f;
			FVector FoliagPos = GetActorLocation() + Pos + Offset;
			FTransform Transform(FoliageRot, FoliagPos);
			TreeFoliageComponent->AddInstance(Transform);
		}
	}
}

void APlanetGenerator::GenerateGrassFoliage(const FVector& Pos, const FVector& Normal, float MinHeightSquared, float MaxHeightSquared) const
{
	if (FMath::FRand() < GrassDensity)
	{
		if(CheckHeigntInRange(MinHeightSquared, MaxHeightSquared, Pos))
		{
			// 1. 노멀 축(Up) 기준으로 랜덤 각도(0~360) 생성
			float RandomYaw = FMath::FRandRange(0.f, 360.f);
	
			// 2. 노멀을 UpVector로 하는 기본 회전
			FQuat AlignQuat = FRotationMatrix::MakeFromZ(Normal).ToQuat();

			// 3. 노멀 축 기준으로 RandomYaw만큼 회전
			FQuat RandomQuat = FQuat(Normal, FMath::DegreesToRadians(RandomYaw));

			// 4. 두 회전 합성 (Random → Align)
			FQuat FinalQuat = RandomQuat * AlignQuat;
			FRotator FoliageRot = FinalQuat.Rotator();

			FVector Offset = Normal * -5.0f;
			FVector FoliagPos = GetActorLocation() + Pos + Offset;
			FTransform Transform(FoliageRot, FoliagPos);

			GrassFoliageComponent->AddInstance(Transform);
		}
	}
}

void APlanetGenerator::GenerateRockFoliage(const FVector& Pos, const FVector& Normal, float MinHeightSquared, float MaxHeightSquared) const
{
}

FVector APlanetGenerator::GetNoisePosition(const FVector& Pos)
{	
	FRandomStream Random(RandomSeed);
	FVector3d Offsets[3];
	for (int k = 0; k < 3; ++k)
	{
		const float RandomOffset = 10000.0f * Random.GetFraction();
		Offsets[k] = FVector3d(RandomOffset, RandomOffset, RandomOffset);
		Offsets[k] += NoiseFrequencyShift;
	}

	// 노이즈 적용
	float Magnitude = PlanetRadius * 0.1f; // 기존 코드와 동일
	FVector3d Displacement;

	for (int32 k = 0; k < 3; ++k)
	{
		FVector NoisePos = (FVector)(NoiseFrequency * (Pos + Offsets[k]));
		Displacement[k] = Magnitude * FMath::PerlinNoise3D(NoiseFrequency * NoisePos);
	}

	FVector3d NoisePos = Pos + Displacement;
	return NoisePos;
}

bool APlanetGenerator::CheckHeigntInRange(float minHeightSquared, float maxHeightSquared, const FVector& Pos) const
{
	float dist = FVector::DistSquared(Pos, GetActorLocation());
	return FMath::IsWithin(dist, minHeightSquared, maxHeightSquared);
}