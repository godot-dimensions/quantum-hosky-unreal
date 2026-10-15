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
			"PhysicsCore",
			"QHGame",
			"Unreal4DCore",
		});
	}
}
