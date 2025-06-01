// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeometryScript/GeometryScriptTypes.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "PlanetGenerateLibrary.generated.h"

class UDynamicMesh;
/**
 * 
 */
UCLASS()
class SPACECAMPER_API UPlanetGenerateLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "GeometryScript|Primitives", meta=(ScriptMethod))
	static UPARAM(DisplayName = "Target Mesh") UDynamicMesh* 
	AppendPlanetSphere( 
		UDynamicMesh* TargetMesh, 
		FGeometryScriptPrimitiveOptions PrimitiveOptions,
		FTransform Transform,
		float Radius = 50,
		int32 StepsPhi = 10,
		int32 StepsTheta = 16,
		EGeometryScriptPrimitiveOriginMode Origin = EGeometryScriptPrimitiveOriginMode::Center,
		UGeometryScriptDebug* Debug = nullptr);
};
