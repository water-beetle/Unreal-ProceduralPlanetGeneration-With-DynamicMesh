// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetUniformFoliage.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"

UPlanetUniformFoliage::UPlanetUniformFoliage()
{
    PrimaryComponentTick.bCanEverTick = true;
	NumChunkSamples = 64;
	CurrentRandom = new FRandomStream(FDateTime::Now().GetTicks());
}

void UPlanetUniformFoliage::CreateFoliageChunk(const FIntPoint& ChunkCoord)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UHierarchicalInstancedStaticMeshComponent* GrassComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(Owner);
    GrassComp->RegisterComponent();
    GrassComp->SetStaticMesh(FoliageMesh);
    GrassComp->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    GrassComp->SetCullDistances(0.f, 50000.f);
    GrassComp->SetCastShadow(false);
	GrassComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);  // 충돌 완전 비활성화
	GrassComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);  // 모든 채널 무시
	GrassComp->bReceivesDecals = false;  // 필요시 데칼도 끄기
	GrassComp->SetCanEverAffectNavigation(false); // 네비게이션 영향 없음 (NavMesh 사용시)

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
    Transforms.Reserve(NumChunkSamples * NumChunkSamples);
	for (int32 i = 0; i < NumChunkSamples; ++i)
	{
		float U = (ChunkCoord.X + (static_cast<float>(i) + 0.5f) / NumChunkSamples) / NumChunks;
		for (int32 j = 0; j < NumChunkSamples; ++j)
		{
			float V = (ChunkCoord.Y + (static_cast<float>(j) + 0.5f) / NumChunkSamples) / NumChunks;

			// 중심 방향에서 LocalOffset을 더해 구 표면으로 투영
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

    GrassComp->AddInstances(Transforms, false);
    GrassComp->BuildTreeIfOutdated(true, true);

    FoliageChunkMap.Add(ChunkCoord, GrassComp);
}

