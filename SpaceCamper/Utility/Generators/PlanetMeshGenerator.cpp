// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetMeshGenerator.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "DynamicMesh/MeshNormals.h"
#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "Planet/Planet.h"


class APlanet;
// Sets default values for this component's properties
UPlanetMeshGenerator::UPlanetMeshGenerator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlanetMeshGenerator::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPlanetMeshGenerator::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

UStaticMesh* UPlanetMeshGenerator::BakeStaticMesh(UStaticMeshDescription* StaticMeshDescription,
	TArray<UMaterialInterface*> Materials)
{
	FString Path = TEXT("/Game/Planet/Meshes/GeneratedPlanets/");
    FString BaseName = TEXT("SM_Planet_");
    int32 Index = 1;
    FString MeshName;
    FString PackageName;

    // AssetRegistry: 기존 에셋 존재 여부 확인
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    // 중복 방지: 이름이 없을 때까지 Index++
    while (true)
    {
        MeshName = FString::Printf(TEXT("%s%d"), *BaseName, Index);
        PackageName = Path + MeshName;

        FSoftObjectPath SoftPath(PackageName + TEXT(".") + MeshName);
        FAssetData ExistingAsset = AssetRegistryModule.Get().GetAssetByObjectPath(SoftPath);

        if (!ExistingAsset.IsValid())
        {
            // 존재하지 않으면 탈출
            break;
        }
        Index++;
    }

    // 혹시 이미 로드된 패키지가 있으면 제거 (중복 충돌 방지)
    UPackage* ExistingPackage = FindPackage(nullptr, *PackageName);
    if (ExistingPackage)
    {
        ExistingPackage->ClearFlags(RF_Standalone | RF_Public);
        ExistingPackage->ConditionalBeginDestroy();
    }

    // 패키지 생성
    UPackage* Package = CreatePackage(*PackageName);
    Package->FullyLoad();

    // StaticMesh 생성
    UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, FName(*MeshName), RF_Public | RF_Standalone);

#if WITH_EDITOR
    if (StaticMeshDescription)
    {
        // 메쉬 빌드
        StaticMesh->BuildFromStaticMeshDescriptions({ StaticMeshDescription });

        // 머티리얼 세팅
    	AActor* Owner = GetOwner();
    	APlanet* PlanetActor = (Owner != nullptr) ? Cast<APlanet>(Owner) : nullptr;
    	if (PlanetActor != nullptr && PlanetActor->PlanetMaterial != nullptr)
    	{
    		StaticMesh->GetStaticMaterials().Add(PlanetActor->PlanetMaterial);		
    	}

        // 기타 설정
        StaticMesh->bAllowCPUAccess = true;
        StaticMesh->NeverStream = true;
        StaticMesh->InitResources();
        StaticMesh->SetLightingGuid();
        StaticMesh->PostEditChange();

        // 패키지 저장 처리
        StaticMesh->MarkPackageDirty();
        FAssetRegistryModule::AssetCreated(StaticMesh);

        // 디스크에 실제로 저장!
        FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
        bool bSuccess = UPackage::SavePackage(
            Package, StaticMesh, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
            *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Successfully baked & saved StaticMesh: %s"), *PackageFileName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to save StaticMesh to disk: %s"), *PackageFileName);
        }

        return StaticMesh;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("StaticMeshDescription is null! Cannot bake mesh."));
        return nullptr;
    }
#else
    // 에디터 전용
    UE_LOG(LogTemp, Warning, TEXT("BakeStaticMesh only works in Editor."));
    return nullptr;
#endif
}

UDynamicMesh* UPlanetMeshGenerator::ApplyPlanetPerlinNoiseToMesh(UDynamicMesh* TargetMesh,
	FGeometryScriptMeshSelection Selection, UGeometryScriptDebug* Debug)
{
	AActor* Owner = GetOwner();
	APlanet* PlanetActor = (Owner != nullptr) ? Cast<APlanet>(Owner) : nullptr;
	if (TargetMesh == nullptr || PlanetActor == nullptr)
	{
		return TargetMesh;
	}

	// Get Actor(Planet)'s Params
	const FRandomStream& Random = PlanetActor->Random;
	float PlanetRadius = PlanetActor->PlanetRadius;
	FVector NoiseFrequencyShift = PlanetActor->NoiseFrequencyShift;
	float NoiseFrequency = PlanetActor->NoiseFrequency;
	
	TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh) 
	{
		FVector3d Offsets[3];
		for (int k = 0; k < 3; ++k)
		{
			//const float RandomOffset = 10000.0f * Random.GetFraction();
			Offsets[k] = FVector3d(0, 0, 0);
			Offsets[k] += (FVector3d)NoiseFrequencyShift;
		}
		
		UE::Geometry::FMeshNormals Normals(&EditMesh);
		auto GetDisplacedPosition = [&EditMesh, &Offsets, &Normals, PlanetRadius, NoiseFrequency](int32 VertexID)
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

