// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class FF44 : ModuleRules
{
	public FF44(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"NavigationSystem",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
            "SlateCore",
            "GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
            "MotionWarping",
            "GameplayCameras",
        });

		PrivateDependencyModuleNames.AddRange(new string[] {
            "MonsterAIPlugIn",
        });
			
        string MonsterAIPath = Path.Combine(ModuleDirectory, "../../Plugins/MonsterAIPlugIn/Source/MonsterAIPlugIn/Public");

        PublicIncludePaths.AddRange(new string[] {
            ModuleDirectory,
			MonsterAIPath,
        });
	}
}
