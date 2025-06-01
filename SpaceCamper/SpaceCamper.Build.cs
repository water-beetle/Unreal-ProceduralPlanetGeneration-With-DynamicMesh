// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SpaceCamper : ModuleRules
{
	public SpaceCamper(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"GeometryScriptingCore",
			"DynamicMesh",
			"ModelingComponents",
			"GeometryFramework",
			"GeometryCore",
			"StaticMeshDescription"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PrivateIncludePaths.AddRange(
		new string[]
		{
			"SpaceCamper" // 프로젝트명
		});
		
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
