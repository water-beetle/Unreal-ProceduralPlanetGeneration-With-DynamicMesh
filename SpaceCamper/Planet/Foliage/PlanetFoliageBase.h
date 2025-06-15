#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlanetFoliageBase.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class APlanet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPACECAMPER_API UPlanetFoliageBase : public UActorComponent
{
        GENERATED_BODY()

public:
        UPlanetFoliageBase();

protected:
        virtual void BeginPlay() override;
        virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                                   FActorComponentTickFunction* ThisTickFunction) override;
        virtual void CreateFoliageChunk(const FIntPoint& ChunkCoord) PURE_VIRTUAL(UProceduralFoliageComponent::CreateFoliageChunk, );

        static FVector2D OctahedralEncode(const FVector& N);
        static FVector  OctahedralDecode(const FVector2D& UV);
        static FIntPoint GetChunkCoordFromOctahedral(const FVector& N, int32 NumChunks);

        void UpdateFoliageChunks(const FIntPoint& CenterChunk);

protected:
        FIntPoint LastCameraChunk = FIntPoint(-1, -1);

        const int32 NumChunks = 32;
        const int32 LoadRange = 3;

        float CurrentRadius;
        float CurrentNoiseFrequency;
        FVector CurrentNoiseShift;
        float MountainHeight;
        float OceanHeight;
        
        
        TMap<FIntPoint, UHierarchicalInstancedStaticMeshComponent*> FoliageChunkMap;

        UPROPERTY(EditAnywhere)
        UStaticMesh* FoliageMesh;
};

