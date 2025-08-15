// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Enemy : ModuleRules
{
    public Enemy(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[] {
            "Enemy/Public",
            "Enemy/Public/Abilities"
        });

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
                "StateTreeModule",
                "GameplayStateTreeModule",
                "UMG",
                "InputCore",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks",
                "AnimGraphRuntime"
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
