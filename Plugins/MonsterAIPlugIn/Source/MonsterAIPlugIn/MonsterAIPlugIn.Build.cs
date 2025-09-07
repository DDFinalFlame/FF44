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
                "GameplayAbilities",   //GAS �ٽ� ���
				"GameplayTags",        //�±� �ý���
				"GameplayTasks",       //Task ��� �ý���
				"GeometryCollectionEngine",
				"FieldSystemEngine","Chaos", 
				"ChaosSolverEngine",        // FChaosPhysicsCollisionInfo / �ݹ� USTRUCT
				"GeometryCollectionEngine", // UGeometryCollectionComponent
				"FieldSystemEngine",         // FieldSystemComponent �� Field ��ü��
                 "Niagara","MotionWarping",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "AIModule",            //AI BehaviorTree, Blackboard ��
				"NavigationSystem",    //�׺���̼� ����� �ʿ��� ���
				"EnhancedInput",
                 "Niagara",//�ʿ� �� �Է� �ý��� (���û���)
				 "Slate",
				"SlateCore",
				"UMG",
				"MotionWarping",
            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
			}
			);
	}
}
