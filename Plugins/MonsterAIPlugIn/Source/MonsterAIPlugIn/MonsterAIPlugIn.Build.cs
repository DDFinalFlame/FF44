// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MonsterAIPlugIn : ModuleRules
{
	public MonsterAIPlugIn(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "GameplayAbilities",   //GAS 핵심 모듈
				"GameplayTags",        //태그 시스템
				"GameplayTasks",       //Task 기반 시스템
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "AIModule",            //AI BehaviorTree, Blackboard 등
				"NavigationSystem",    //네비게이션 기능이 필요한 경우
				"EnhancedInput",       //필요 시 입력 시스템 (선택사항)
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
			}
			);
	}
}
