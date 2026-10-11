using UnrealBuildTool;

public class QHGame : ModuleRules
{
	public QHGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Unreal4DCore",
		});
	}
}
