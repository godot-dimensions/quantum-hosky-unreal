using UnrealBuildTool;

public class QHGame : ModuleRules
{
	public QHGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"AngelscriptCode",
			"AngelscriptEnhancedInput",
			"Chaos",
			"ChaosCore",
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"Engine",
			"EnhancedInput",
			"GameplayTags",
			"InputCore",
			"UMG",
			"Unreal4DCore",
		});

		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
	}
}
