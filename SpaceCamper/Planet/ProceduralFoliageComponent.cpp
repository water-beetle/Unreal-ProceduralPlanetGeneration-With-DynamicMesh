#include "ProceduralFoliageComponent.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Planet/Planet.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

UProceduralFoliageComponent::UProceduralFoliageComponent()
{
        PrimaryComponentTick.bCanEverTick = true;
}

void UProceduralFoliageComponent::BeginPlay()
{
        Super::BeginPlay();

        APlanet* PlanetOwner = Cast<APlanet>(GetOwner());
        if (PlanetOwner)
        {
                CurrentRadius = PlanetOwner->PlanetRadius;
                CurrentNoiseFrequency = PlanetOwner->NoiseFrequency;
                CurrentNoiseShift = PlanetOwner->NoiseFrequencyShift * 10000.0f;
                CurrentRandom = &(PlanetOwner->Random);
        }
}

void UProceduralFoliageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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
                UpdateFoliageChunks(CurrentChunk);
        }
}

FVector2D UProceduralFoliageComponent::OctahedralEncode(const FVector& N)
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

FVector UProceduralFoliageComponent::OctahedralDecode(const FVector2D& UV)
{
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
                        -1.0f);
        }

        return N.GetSafeNormal();
}

FIntPoint UProceduralFoliageComponent::GetChunkCoordFromOctahedral(const FVector& N, int32 NumChunks)
{
        FVector2D Oct = OctahedralEncode(N);
        FVector2D UV = (Oct * 0.5f) + FVector2D(0.5f, 0.5f);

        int32 X = FMath::Clamp(FMath::FloorToInt(UV.X * NumChunks), 0, NumChunks - 1);
        int32 Y = FMath::Clamp(FMath::FloorToInt(UV.Y * NumChunks), 0, NumChunks - 1);
        return FIntPoint(X, Y);
}

void UProceduralFoliageComponent::UpdateFoliageChunks(const FIntPoint& CenterChunk)
{
        TSet<FIntPoint> VisibleChunks;

        for (int32 dx = -LoadRange; dx <= LoadRange; ++dx)
        {
                for (int32 dy = -LoadRange; dy <= LoadRange; ++dy)
                {
                        int32 X = (CenterChunk.X + dx + NumChunks) % NumChunks;
                        int32 Y = (CenterChunk.Y + dy + NumChunks) % NumChunks;

                        FIntPoint ChunkCoord(X, Y);
                        VisibleChunks.Add(ChunkCoord);

                        if (!FoliageChunkMap.Contains(ChunkCoord))
                        {
                                CreateFoliageChunk(ChunkCoord);
                        }
                }
        }

        TArray<FIntPoint> ToRemove;
        for (const auto& Pair : FoliageChunkMap)
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

        for (const FIntPoint& Key : ToRemove)
        {
                FoliageChunkMap.Remove(Key);
        }
}

