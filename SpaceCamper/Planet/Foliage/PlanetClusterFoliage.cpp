// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetClusterFoliage.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"


// Sets default values for this component's properties
UPlanetClusterFoliage::UPlanetClusterFoliage()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	FGuid Guid = FGuid::NewGuid();
	int32 Seed = GetTypeHash(Guid);
	CurrentRandom = new FRandomStream(Seed);
}


// Called when the game starts
void UPlanetClusterFoliage::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPlanetClusterFoliage::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlanetClusterFoliage::CreateFoliageChunk(const FIntPoint& ChunkCoord)
{
	AActor* Owner = GetOwner();
    if (!Owner) return;

	if(!FoliageMesh) return;

	

    UHierarchicalInstancedStaticMeshComponent* FlowerComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(Owner);
    FlowerComp->RegisterComponent();
    FlowerComp->SetStaticMesh(FoliageMesh);
    FlowerComp->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    FlowerComp->SetCullDistances(0.f, 50000.f);
    FlowerComp->SetCastShadow(false);
	FlowerComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);  // 충돌 완전 비활성화
	FlowerComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);  // 모든 채널 무시
	FlowerComp->bReceivesDecals = false;  // 필요시 데칼도 끄기
	FlowerComp->SetCanEverAffectNavigation(false); // 네비게이션 영향 없음 (NavMesh 사용시)

    // 노이즈 시프트만 적용한 오프셋
    FVector3d Offsets[3];
    for (int32 k = 0; k < 3; ++k)
    {
        Offsets[k] = (FVector3d)CurrentNoiseShift;
    }

    // Chunk 중심 방향 계산 (Octahedral 역 변환)
    FVector2D CenterUV(
        (ChunkCoord.X + 0.5f) / NumChunks,
        (ChunkCoord.Y + 0.5f) / NumChunks
    );
	FVector N = OctahedralDecode(CenterUV);
	

    TArray<FTransform> Transforms;
    int32 ClusterCount = CurrentRandom->RandRange(MinClusters, MaxClusters);
    Transforms.Reserve(ClusterCount * ClusterSize);

    for (int32 c = 0; c < ClusterCount; ++c)
    {
        float CenterU = (ChunkCoord.X + CurrentRandom->FRand()) / NumChunks;
        float CenterV = (ChunkCoord.Y + CurrentRandom->FRand()) / NumChunks;

        for (int32 i = 0; i < ClusterSize; ++i)
        {
            float OffsetU = CurrentRandom->FRandRange(-ClusterSpread, ClusterSpread) / NumChunks;
            float OffsetV = CurrentRandom->FRandRange(-ClusterSpread, ClusterSpread) / NumChunks;

            float U = CenterU + OffsetU;
            float V = CenterV + OffsetV;

            FVector PointOnSphere = OctahedralDecode(FVector2D(U, V)) * CurrentRadius;

            float Magnitude = CurrentRadius * 0.1f;
            FVector3d Displacement;
            for (int32 k = 0; k < 3; ++k)
            {
                FVector NoisePos = (PointOnSphere + Offsets[k]) * CurrentNoiseFrequency;
                Displacement[k] = Magnitude * FMath::PerlinNoise3D(NoisePos * CurrentNoiseFrequency);
            }
            FVector NoisePoint = PointOnSphere + Displacement;

            FQuat BaseQuat = FRotationMatrix::MakeFromZ(NoisePoint).ToQuat();
            float RandYaw = CurrentRandom->FRandRange(0.f, 360.f);
            FQuat RandomQuat = FQuat(NoisePoint.GetSafeNormal(), FMath::DegreesToRadians(RandYaw));
            FRotator Rot = (RandomQuat * BaseQuat).Rotator();

            Transforms.Add(FTransform(Rot, NoisePoint));
        }
    }

    FlowerComp->AddInstances(Transforms, false);
    FlowerComp->BuildTreeIfOutdated(true, true);

    FoliageChunkMap.Add(ChunkCoord, FlowerComp);
}

