using UnrealBuildTool;

public class QHEditor : ModuleRules
{
	public QHEditor(ReadOnlyTargetRules Target) : base(Target)
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
