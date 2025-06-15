// Fill out your copyright notice in the Description page of Project Settings.


#include "GrassFoliage.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Planet/Planet.h"


// Sets default values for this component's properties
UGrassFoliage::UGrassFoliage()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGrassFoliage::BeginPlay()
{
    Super::BeginPlay();

    APlanet* PlanetOwner = Cast<APlanet>(GetOwner());
	if (PlanetOwner)
	{
		CurrentRadius = PlanetOwner->PlanetRadius;
		CurrentNoiseFrequency = PlanetOwner->NoiseFrequency;
		CurrentNoiseShift = PlanetOwner->NoiseFrequencyShift;
		CurrentRandom = &(PlanetOwner->Random);
	}
}


// Called every frame
void UGrassFoliage::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Owner = GetOwner();
    if (!Owner) return;

	APlayerController* PC = Owner->GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FVector FromCenter = (CameraLocation - Owner->GetActorLocation()).GetSafeNormal();

	FIntPoint CurrentChunk = GetChunkCoordFromOctahedral(FromCenter, NumChunks);

	if (CurrentChunk != LastCameraChunk)
	{
		LastCameraChunk = CurrentChunk;
		UpdateGrassChunks(CurrentChunk);
	}
}

FVector2D UGrassFoliage::OctahedralEncode(const FVector& N)
{
	FVector n = N / (FMath::Abs(N.X) + FMath::Abs(N.Y) + FMath::Abs(N.Z));
	if (n.Z < 0)
	{
		float x = (1.0f - FMath::Abs(n.Y)) * (n.X >= 0.0f ? 1.0f : -1.0f);
		float y = (1.0f - FMath::Abs(n.X)) * (n.Y >= 0.0f ? 1.0f : -1.0f);
		return FVector2D(x, y);
	}
	return FVector2D(n.X, n.Y);
}

FIntPoint UGrassFoliage::GetChunkCoordFromOctahedral(const FVector& N, int32 NumChunks)
{
	FVector2D Oct = OctahedralEncode(N);
	// Oct 좌표계 [-1,1] → [0,1]로 변환
	FVector2D UV = (Oct * 0.5f) + FVector2D(0.5f, 0.5f);

	int32 X = FMath::Clamp(FMath::FloorToInt(UV.X * NumChunks), 0, NumChunks - 1);
	int32 Y = FMath::Clamp(FMath::FloorToInt(UV.Y * NumChunks), 0, NumChunks - 1);
	return FIntPoint(X, Y);
}

void UGrassFoliage::UpdateGrassChunks(const FIntPoint& CenterChunk)
{
	const int32 LoadRange = 2;

	// 이번 프레임에서 필요한 Chunk 좌표만 저장
	TSet<FIntPoint> VisibleChunks;

	for (int32 dx = -LoadRange; dx <= LoadRange; ++dx)
	{
		for (int32 dy = -LoadRange; dy <= LoadRange; ++dy)
		{
			int32 X = (CenterChunk.X + dx + NumChunks) % NumChunks; // 음수 방지
			int32 Y = (CenterChunk.Y + dy + NumChunks) % NumChunks; // 음수 방지

			FIntPoint ChunkCoord(X, Y);
			VisibleChunks.Add(ChunkCoord);

			if (!GrassChunkMap.Contains(ChunkCoord))
			{
				CreateGrassChunk(ChunkCoord);
			}
		}
	}

	// 안 보이는 Chunk 삭제
	TArray<FIntPoint> ToRemove;
	for (const auto& Pair : GrassChunkMap)
	{
		if (!VisibleChunks.Contains(Pair.Key))
		{
			if (Pair.Value)
			{
				Pair.Value->DestroyComponent();
			}
			ToRemove.Add(Pair.Key);
		}
	}

	// 맵에서 제거
	for (const FIntPoint& Key : ToRemove)
	{
		GrassChunkMap.Remove(Key);
	}
}

void UGrassFoliage::CreateGrassChunk(const FIntPoint& ChunkCoord)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UHierarchicalInstancedStaticMeshComponent* GrassComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(Owner);
    GrassComp->RegisterComponent();
    GrassComp->SetStaticMesh(GrassFoliageMesh);
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
    FVector2D UV(
        (ChunkCoord.X + 0.5f) / NumChunks,
        (ChunkCoord.Y + 0.5f) / NumChunks
    );
    FVector2D Oct = UV * 2.0f - FVector2D(1.0f, 1.0f);

    FVector N;
    if (1.0f - FMath::Abs(Oct.X) - FMath::Abs(Oct.Y) >= 0.0f)
    {
        N = FVector(Oct.X, Oct.Y, 1.0f - FMath::Abs(Oct.X) - FMath::Abs(Oct.Y));
    }
    else
    {
        N = FVector(
            Oct.X >= 0.0f ? 1.0f - FMath::Abs(Oct.Y) : -1.0f + FMath::Abs(Oct.Y),
            Oct.Y >= 0.0f ? 1.0f - FMath::Abs(Oct.X) : -1.0f + FMath::Abs(Oct.X),
            -1.0f
        );
    }
    N.Normalize();

    FVector Arbitrary = (FMath::Abs(N.X) < 0.99f) ? FVector(1, 0, 0) : FVector(0, 1, 0);
    FVector T1 = FVector::CrossProduct(N, Arbitrary).GetSafeNormal();
    FVector T2 = FVector::CrossProduct(N, T1).GetSafeNormal();

	// 대략 Chunk 중심각 (라디안) = PI / NumChunks (반구 영역의 각도 크기)
    float ChunkAngleRad = PI / NumChunks;              // 중심각 (라디안)
    float ChunkLength = CurrentRadius * ChunkAngleRad; // Chunk 대략적인 실제 크기 (m)
    float PatchDelta = ChunkLength / NumChunkSamples;       // 각 샘플 간 Tangent 공간 거리

	// Tangent 공간에서 delta 단위로 샘플링
	float HalfSize = (NumChunkSamples / 2) * PatchDelta;

    TArray<FTransform> Transforms;
    Transforms.Reserve(NumChunkSamples * NumChunkSamples);
	for (int32 i = 0; i < NumChunkSamples; ++i)
	{
		float u = -HalfSize + i * PatchDelta;
		for (int32 j = 0; j < NumChunkSamples; ++j)
		{
			float v = -HalfSize + j * PatchDelta;

			// Tangent 공간에서의 위치 오프셋
			FVector LocalOffset = u * T1 + v * T2;

			// 중심 방향에서 LocalOffset을 더해 구 표면으로 투영
            FVector PointOnSphere = (N * CurrentRadius + LocalOffset).GetSafeNormal() * CurrentRadius;

            float Magnitude = CurrentRadius * 0.1f;
            FVector3d Displacement;
            for (int32 k = 0; k < 3; ++k)
            {
                    FVector NoisePos = (PointOnSphere + Offsets[k]) * CurrentNoiseFrequency;
                    Displacement[k] = Magnitude * FMath::PerlinNoise3D(NoisePos);
            }
            FVector NoisePoint = PointOnSphere + Displacement;

            FRotator Rot = FRotationMatrix::MakeFromZ(NoisePoint).Rotator();

            Transforms.Add(FTransform(Rot, NoisePoint));
        }
    }

    GrassComp->AddInstances(Transforms, false);
    GrassComp->BuildTreeIfOutdated(true, true);

    GrassChunkMap.Add(ChunkCoord, GrassComp);
}

