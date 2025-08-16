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
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"MonsterAIPlugIn",
			"MotionWarping",
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });
			
        string MonsterAIPath = Path.Combine(ModuleDirectory, "../../Plugins/MonsterAIPlugIn/Source/MonsterAIPlugIn/Public");

        PublicIncludePaths.AddRange(new string[] {
            ModuleDirectory,
			MonsterAIPath
        });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
