// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project_Gear : ModuleRules
{
	public Project_Gear(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
