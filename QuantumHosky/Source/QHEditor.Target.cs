using UnrealBuildTool;

public class QHEditorTarget : TargetRules
{
	public QHEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("QHGame");
		ExtraModuleNames.Add("QHEditor");
	}
}
