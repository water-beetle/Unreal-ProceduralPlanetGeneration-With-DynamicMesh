// Fill out your copyright notice in the Description page of Project Settings.

#include "PlanetGenerateLibrary.h"

#include "DynamicMeshEditor.h"
#include "PlanetSphereGenerator.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "UDynamicMesh.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "DynamicMesh/MeshTransforms.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "Generators/SphereGenerator.h"

using namespace UE::Geometry;

static void ApplyPrimitiveOptionsToMesh(
	FDynamicMesh3& Mesh, const FTransform& Transform, 
	FGeometryScriptPrimitiveOptions PrimitiveOptions, 
	FVector3d PreTranslate = FVector3d::Zero(),
	TOptional<FQuaterniond> PreRotate = TOptional<FQuaterniond>())
{
	bool bHasTranslate = PreTranslate.SquaredLength() > 0;
	if (PreRotate.IsSet())
	{
		FFrame3d Frame(PreTranslate, *PreRotate);
		MeshTransforms::FrameCoordsToWorld(Mesh, Frame);
	}
	else if (bHasTranslate)
	{
		MeshTransforms::Translate(Mesh, PreTranslate);
	}

	MeshTransforms::ApplyTransform(Mesh, (FTransformSRT3d)Transform, true);
	if (PrimitiveOptions.PolygroupMode == EGeometryScriptPrimitivePolygroupMode::SingleGroup)
	{
		for (int32 tid : Mesh.TriangleIndicesItr())
		{
			Mesh.SetTriangleGroup(tid, 0);
		}
	}
	if (PrimitiveOptions.bFlipOrientation)
	{
		Mesh.ReverseOrientation(true);
		if (Mesh.HasAttributes())
		{
			FDynamicMeshNormalOverlay* Normals = Mesh.Attributes()->PrimaryNormals();
			for (int elemid : Normals->ElementIndicesItr())
			{
				Normals->SetElement(elemid, -Normals->GetElement(elemid));
			}
		}
	}
}

static void AppendPrimitive(
	UDynamicMesh* TargetMesh, 
	FMeshShapeGenerator* Generator, 
	FTransform Transform, 
	FGeometryScriptPrimitiveOptions PrimitiveOptions,
	FVector3d PreTranslate = FVector3d::Zero(),
	TOptional<FQuaterniond> PreRotate = TOptional<FQuaterniond>())
{
	if (TargetMesh->IsEmpty())
	{
		TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh)
		{
			EditMesh.Copy(Generator);
			ApplyPrimitiveOptionsToMesh(EditMesh, Transform, PrimitiveOptions, PreTranslate, PreRotate);
		}, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);
	}
	else
	{
		FDynamicMesh3 TempMesh(Generator);
		ApplyPrimitiveOptionsToMesh(TempMesh, Transform, PrimitiveOptions, PreTranslate, PreRotate);
		TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh)
		{
			FMeshIndexMappings TmpMappings;
			FDynamicMeshEditor Editor(&EditMesh);
			Editor.AppendMesh(&TempMesh, TmpMappings);
		}, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);
	}
}

static void AppendPrimitiveWithVertexIndexMap(
	UDynamicMesh* TargetMesh,
	TArray<int32>& OutVertexIndexMap,
	FMeshShapeGenerator* Generator,
	FTransform Transform,
	FGeometryScriptPrimitiveOptions PrimitiveOptions,
	FVector3d PreTranslate = FVector3d::Zero())
{
	if (TargetMesh->IsEmpty())
	{
		TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh)
		{
			EditMesh.Copy(Generator);
			ApplyPrimitiveOptionsToMesh(EditMesh, Transform, PrimitiveOptions, PreTranslate);
			// For a clean/empty mesh, the vertex map is just an identity map
			OutVertexIndexMap.SetNum(EditMesh.VertexCount());
			for (int32 Idx = 0; Idx < OutVertexIndexMap.Num(); ++Idx)
			{
				OutVertexIndexMap[Idx] = Idx;
			}
		}, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);
	}
	else
	{
		FDynamicMesh3 TempMesh(Generator);
		ApplyPrimitiveOptionsToMesh(TempMesh, Transform, PrimitiveOptions, PreTranslate);
		TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh)
		{
			FMeshIndexMappings TmpMappings;
			FDynamicMeshEditor Editor(&EditMesh);
			Editor.AppendMesh(&TempMesh, TmpMappings);
			OutVertexIndexMap.Init(INDEX_NONE, Generator->Vertices.Num());
			for (TPair<int32, int32> IndexToID : TmpMappings.GetVertexMap().GetForwardMap())
			{
				OutVertexIndexMap[IndexToID.Key] = IndexToID.Value;
			}
		}, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);
	}
}

static void AppendPrimitiveMesh(
	UDynamicMesh* TargetMesh, 
	FDynamicMesh3& AppendMesh,
	FTransform Transform, 
	FGeometryScriptPrimitiveOptions PrimitiveOptions,
	FVector3d PreTranslate = FVector3d::Zero())
{
	auto ApplyOptionsToMesh = [&Transform, &PrimitiveOptions, PreTranslate](FDynamicMesh3& Mesh)
	{
		if (PreTranslate.SquaredLength() > 0)
		{
			MeshTransforms::Translate(Mesh, PreTranslate);
		}

		MeshTransforms::ApplyTransform(Mesh, (FTransformSRT3d)Transform, true);
		if (PrimitiveOptions.PolygroupMode == EGeometryScriptPrimitivePolygroupMode::SingleGroup)
		{
			for (int32 tid : Mesh.TriangleIndicesItr())
			{
				Mesh.SetTriangleGroup(tid, 0);
			}
		}
		if (PrimitiveOptions.bFlipOrientation)
		{
			Mesh.ReverseOrientation(true);
			if (Mesh.HasAttributes())
			{
				FDynamicMeshNormalOverlay* Normals = Mesh.Attributes()->PrimaryNormals();
				for (int elemid : Normals->ElementIndicesItr())
				{
					Normals->SetElement(elemid, -Normals->GetElement(elemid));
				}
			}
		}
	};

	if (TargetMesh->IsEmpty())
	{
		TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh)
		{
			EditMesh.Copy(AppendMesh);
			ApplyOptionsToMesh(EditMesh);
		}, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);
	}
	else
	{
		ApplyOptionsToMesh(AppendMesh);
		TargetMesh->EditMesh([&](FDynamicMesh3& EditMesh)
		{
			FMeshIndexMappings TmpMappings;
			FDynamicMeshEditor Editor(&EditMesh);
			Editor.AppendMesh(&AppendMesh, TmpMappings);
		}, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown, false);
	}
}

UDynamicMesh* UPlanetGenerateLibrary::AppendPlanetSphere(
	UDynamicMesh* TargetMesh,
	FGeometryScriptPrimitiveOptions PrimitiveOptions,
	FTransform Transform,
	float Radius,
	int32 StepsPhi,
	int32 StepsTheta,
	EGeometryScriptPrimitiveOriginMode Origin,
	UGeometryScriptDebug* Debug)
{
	if (TargetMesh == nullptr)
	{
		return TargetMesh;
	}

	FPlanetSphereGenerator SphereGenerator;
	SphereGenerator.Radius = FMath::Max(FMathf::ZeroTolerance, Radius);
	SphereGenerator.NumPhi = FMath::Max(3, StepsPhi);
	SphereGenerator.NumTheta = FMath::Max(3, StepsTheta);
	SphereGenerator.Generate();

	FVector3d OriginShift = FVector3d::Zero();
	AppendPrimitive(TargetMesh, &SphereGenerator, Transform, PrimitiveOptions, OriginShift);

	return TargetMesh;
}
