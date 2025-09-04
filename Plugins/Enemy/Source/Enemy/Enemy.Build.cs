// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Enemy : ModuleRules
{
    public Enemy(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(
            new string[] {
            }
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "AIModule",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks",
                "AnimGraphRuntime",
                "MonsterAIPlugIn",
                "Niagara"
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
            );
    }
}
