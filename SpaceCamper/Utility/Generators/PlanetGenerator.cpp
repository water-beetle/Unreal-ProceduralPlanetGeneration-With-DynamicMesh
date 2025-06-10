// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetGenerator.h"
#include "DynamicMeshEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "DynamicMesh/MeshNormals.h"


// Sets default values for this component's properties
APlanetGenerator::APlanetGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	
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

void APlanetGenerator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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

	float OceanHeightSquared = OceanHeight * OceanHeight;
	float MountainHeightSquared = MountainHeight * MountainHeight;
	
	DynamicMesh->ProcessMesh([&](const FDynamicMesh3& Mesh)
	{
		UE::Geometry::FMeshNormals Normals(&Mesh);
		Normals.ComputeVertexNormals();

		PlanetNormals = Normals;
		
		for (int32 VertexID : Mesh.VertexIndicesItr())
		{
			FVector3d Pos = Mesh.GetVertex(VertexID);
			FVector3d Normal = Normals[VertexID];
			
			if(TreeFoliageMesh)
			{
				// 나무 위치를 Normal Vector에 수직으로 하니 어색함 -> 그냥 위치로 하는게 더 보기좋음
				GenerateTreeFoliage(Pos, Normal, OceanHeightSquared, MountainHeightSquared);
			}
		}
	});

	GenerateGrassFoliage(OceanHeightSquared, MountainHeightSquared);
}

void APlanetGenerator::UpdateGrassChunks(const FIntPoint& CenterChunk)
{
	const int32 LoadRange = 1;

	for (int32 dLat = -LoadRange; dLat <= LoadRange; ++dLat)
	{
		for (int32 dLon = -LoadRange; dLon <= LoadRange; ++dLon)
		{
			int32 LatIndex = FMath::Clamp(CenterChunk.X + dLat, 0, 180 / LatitudeStep - 1);
			int32 LonIndex = (CenterChunk.Y + dLon + TotalLonSteps) % TotalLonSteps;

			FIntPoint ChunkCoord(LatIndex, LonIndex);

			if (!GrassChunkMap.Contains(ChunkCoord))
			{
				CreateGrassChunk(ChunkCoord);
			}
		}
	}

	// 먼 Chunk 제거
	TArray<FIntPoint> ToRemove;
	for (const auto& Pair : GrassChunkMap)
	{
		FIntPoint Chunk = Pair.Key;
		int32 LatDist = FMath::Abs(Chunk.X - CenterChunk.X);
		int32 LonDist = FMath::Abs(Chunk.Y - CenterChunk.Y);
		if (LatDist > LoadRange || LonDist > LoadRange)
		{
			ToRemove.Add(Chunk);
		}
	}

	for (const FIntPoint& Chunk : ToRemove)
	{
		if (UHierarchicalInstancedStaticMeshComponent* Comp = GrassChunkMap[Chunk])
		{
			Comp->DestroyComponent();
		}
		GrassChunkMap.Remove(Chunk);
	}
}

void APlanetGenerator::CreateGrassChunk(const FIntPoint& ChunkCoord)
{
    UHierarchicalInstancedStaticMeshComponent* GrassComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
    GrassComp->RegisterComponent();
    GrassComp->SetStaticMesh(GrassFoliageMesh);
    GrassComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    GrassComp->SetCullDistances(0.f, 5000.f);
    GrassComp->SetCastShadow(false);

    // 랜덤 오프셋
    FRandomStream Random(RandomSeed);
    FVector3d Offsets[3];
    for (int32 k = 0; k < 3; ++k)
    {
        float RandomOffset = 10000.0f * Random.GetFraction();
        Offsets[k] = FVector3d(RandomOffset, RandomOffset, RandomOffset) + NoiseFrequencyShift;
    }

    // Chunk 중심 위도/경도로 중심 방향 계산
    float LatCenter = (ChunkCoord.X + 0.5f) * LatitudeStep;
    float LonCenter = (ChunkCoord.Y + 0.5f) * LongitudeStep - 180.f;

    FVector N = FVector(
        FMath::Sin(FMath::DegreesToRadians(LatCenter)) * FMath::Cos(FMath::DegreesToRadians(LonCenter)),
        FMath::Sin(FMath::DegreesToRadians(LatCenter)) * FMath::Sin(FMath::DegreesToRadians(LonCenter)),
        FMath::Cos(FMath::DegreesToRadians(LatCenter))
    ).GetSafeNormal();

    FVector Arbitrary = (FMath::Abs(N.X) < 0.99f) ? FVector(1, 0, 0) : FVector(0, 1, 0);
    FVector T1 = FVector::CrossProduct(N, Arbitrary).GetSafeNormal();
    FVector T2 = FVector::CrossProduct(N, T1).GetSafeNormal();

    // Chunk 반경 (ThetaMax) 설정
    float ThetaMaxDeg = LatitudeStep * 0.6f; // Chunk 영역 대략 범위 (자유롭게 조정 가능)
    float ThetaMax = FMath::DegreesToRadians(ThetaMaxDeg);

    // 샘플 밀도
    constexpr int32 NumSamples = 20;

    TArray<FTransform> Transforms;
    for (int32 i = 0; i < NumSamples; ++i)
    {
        float u = -1.0f + 2.0f * i / (NumSamples - 1);
        float Theta = u * ThetaMax;

        for (int32 j = 0; j < NumSamples; ++j)
        {
            float v = -1.0f + 2.0f * j / (NumSamples - 1);
            float Phi = v * ThetaMax;

            // 로컬 방향 벡터
            FVector LocalDir = (T1 * FMath::Cos(Phi) + T2 * FMath::Sin(Phi)).GetSafeNormal();

            // 최종 위치
            FVector PointOnSphere = (N * FMath::Cos(Theta) + LocalDir * FMath::Sin(Theta)) * PlanetRadius;
            FVector NoisePoint = GetNoisePosition(PointOnSphere, Offsets);

            if (!CheckHeightInRange(OceanHeight * OceanHeight + 30, MountainHeight * MountainHeight, NoisePoint))
                continue;

            FRotator Rot = FRotationMatrix::MakeFromZ(NoisePoint).Rotator();
            Transforms.Add(FTransform(Rot, NoisePoint));
        }
    }

    GrassComp->AddInstances(Transforms, false);
    GrassComp->BuildTreeIfOutdated(true, true);

    GrassChunkMap.Add(ChunkCoord, GrassComp);

    UE_LOG(LogTemp, Log, TEXT("Chunk (%d, %d): %d grass instances"), ChunkCoord.X, ChunkCoord.Y, Transforms.Num());
}

void APlanetGenerator::GenerateTreeFoliage(const FVector& Pos, const FVector& Normal, float MinHeightSquared, float MaxHeightSquared) const
{
	if (FMath::FRand() < TreeDensity)
	{
		if(CheckHeightInRange(MinHeightSquared + 30, MaxHeightSquared, Pos)) // 가끔씩 바다랑 겹치는 경우가 있어서 +30 더해줌
		{
			FRotator FoliageRot = FRotationMatrix::MakeFromZ(Pos).Rotator();
			FVector Offset = Normal * -50.0f;
			FVector FoliagPos = GetActorLocation() + Pos + Offset;
			FTransform Transform(FoliageRot, FoliagPos);
			TreeFoliageComponent->AddInstance(Transform);
		}
	}
}

void APlanetGenerator::GenerateGrassFoliage(float MinHeightSquared, float MaxHeightSquared) const
{
    // const float GoldenRatio = (1.f + FMath::Sqrt(5.f)) / 2.f;
    // constexpr int32 NumPoints = 1000000;
    // const float TwoPiOverGolden = 2.f * PI / GoldenRatio;
    //
    // // 랜덤 오프셋 생성
    // FRandomStream Random(RandomSeed);
    // FVector3d Offsets[3];
    // for (int32 k = 0; k < 3; ++k)
    // {
    //     float RandomOffset = 10000.0f * Random.GetFraction();
    //     Offsets[k] = FVector3d(RandomOffset, RandomOffset, RandomOffset) + NoiseFrequencyShift;
    // }
    //
    // // 각 풀 Transform을 고유 인덱스에 미리 할당
    // TArray<FTransform> InstanceTransforms;
    // InstanceTransforms.SetNumUninitialized(NumPoints);
    //
    // // 병렬로 각 인덱스별 Transform 계산
    // ParallelFor(NumPoints, [this, MinHeightSquared, MaxHeightSquared, Offsets, TwoPiOverGolden, NumPoints, &InstanceTransforms](int32 i)
    // {
    //     float theta = i * TwoPiOverGolden;
    //     float z = 1.f - (2.f * i) / (NumPoints - 1);
    //     float radius = FMath::Sqrt(1.f - z * z);
    //     float x = FMath::Cos(theta) * radius;
    //     float y = FMath::Sin(theta) * radius;
    //
    //     FVector PointOnSphere = FVector(x, y, z) * PlanetRadius;
    //     FVector NoisePointOnSphere = GetNoisePosition(PointOnSphere, Offsets);
    //
    //     if (CheckHeigntInRange(MinHeightSquared + 30, MaxHeightSquared, NoisePointOnSphere))
    //     {
    //         FRotator FoliageRot = FRotationMatrix::MakeFromZ(NoisePointOnSphere).Rotator();
    //         InstanceTransforms[i] = FTransform(FoliageRot, NoisePointOnSphere);
    //     }
    //     else
    //     {
    //         InstanceTransforms[i] = FTransform::Identity;
    //     }
    // });
    //
    // // Identity 아닌 Transform만 추려서 최종 리스트로
    // TArray<FTransform> FinalTransforms;
    // FinalTransforms.Reserve(NumPoints);
    //
    // for (const FTransform& Transform : InstanceTransforms)
    // {
    //     if (!Transform.Equals(FTransform::Identity))
    //     {
    //         FinalTransforms.Add(Transform);
    //     }
    // }
	   //
    // GrassFoliageComponent->SetCullDistances(0.f, 5000.f); // 필요에 따라 Cull 거리 세팅
    //
    // // 한 번에 풀 인스턴스 추가
    // GrassFoliageComponent->AddInstances(FinalTransforms, false);
    // // 마지막으로 트리 재생성
    // GrassFoliageComponent->BuildTreeIfOutdated(true, true);

	// constexpr int32 NumLatitude = 500;    // 위도 단계
 //    constexpr int32 NumLongitude = 500;  // 경도 단계
 //    const int32 NumPoints = NumLatitude * NumLongitude;
 //
 //    // 랜덤 오프셋 생성
 //    FRandomStream Random(RandomSeed);
 //    FVector3d Offsets[3];
 //    for (int32 k = 0; k < 3; ++k)
 //    {
 //        float RandomOffset = 10000.0f * Random.GetFraction();
 //        Offsets[k] = FVector3d(RandomOffset, RandomOffset, RandomOffset) + NoiseFrequencyShift;
 //    }
	//
 //    TArray<FTransform> InstanceTransforms;
 //    InstanceTransforms.SetNumUninitialized(NumPoints);
 //
 //    // 병렬로 각 인덱스별 Transform 계산
 //    ParallelFor(NumPoints, [this, MinHeightSquared, MaxHeightSquared, Offsets, NumLatitude, NumLongitude, &InstanceTransforms](int32 idx)
 //    {
 //        int32 lat = idx / NumLongitude;
 //        int32 lon = idx % NumLongitude;
 //
 //        float v = lat / float(NumLatitude - 1);
 //        float theta = v * PI;
 //
 //        float u = lon / float(NumLongitude);
 //        float phi = u * 2.f * PI;
 //
 //        float x = FMath::Sin(theta) * FMath::Cos(phi);
 //        float y = FMath::Sin(theta) * FMath::Sin(phi);
 //        float z = FMath::Cos(theta);
 //
 //        FVector PointOnSphere = FVector(x, y, z) * PlanetRadius;
 //        FVector NoisePointOnSphere = GetNoisePosition(PointOnSphere, Offsets);
 //
 //        if (CheckHeigntInRange(MinHeightSquared + 30, MaxHeightSquared, NoisePointOnSphere))
 //        {
 //            FRotator FoliageRot = FRotationMatrix::MakeFromZ(NoisePointOnSphere).Rotator();
 //            InstanceTransforms[idx] = FTransform(FoliageRot, NoisePointOnSphere);
 //        }
 //        else
 //        {
 //            InstanceTransforms[idx] = FTransform::Identity;
 //        }
 //    });
 //
 //    // Identity 아닌 Transform만 추려서 최종 리스트로
 //    TArray<FTransform> FinalTransforms;
 //    FinalTransforms.Reserve(NumPoints);
 //
 //    for (const FTransform& Transform : InstanceTransforms)
 //    {
 //        if (!Transform.Equals(FTransform::Identity))
 //        {
 //            FinalTransforms.Add(Transform);
 //        }
 //    }
 //
	// GrassFoliageComponent->SetCastShadow(false);
 //    GrassFoliageComponent->SetCullDistances(0.f, 5000.f);
 //
 //    // 한 번에 풀 인스턴스 추가
 //    GrassFoliageComponent->AddInstances(FinalTransforms, false);
 //    // 마지막으로 트리 재생성
 //    GrassFoliageComponent->BuildTreeIfOutdated(true, true);
}

void APlanetGenerator::GenerateRockFoliage(const FVector& Pos, const FVector& Normal, float MinHeightSquared, float MaxHeightSquared) const
{
	if (FMath::FRand() < RockDensity)
	{
		if(CheckHeightInRange(MinHeightSquared, MaxHeightSquared, Pos))
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

			RockFoliageComponent->AddInstance(Transform);
		}
	}
}

FVector APlanetGenerator::GetNoisePosition(const FVector& Pos, const FVector3d Offsets[3]) const
{	
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

bool APlanetGenerator::CheckHeightInRange(float minHeightSquared, float maxHeightSquared, const FVector& Pos) const
{
	float dist = FVector::DistSquared(Pos, GetActorLocation());
	return FMath::IsWithin(dist, minHeightSquared, maxHeightSquared);
}