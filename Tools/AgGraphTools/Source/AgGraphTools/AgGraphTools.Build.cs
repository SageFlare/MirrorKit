using UnrealBuildTool;

public class AgGraphTools : ModuleRules
{
	public AgGraphTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "BlueprintGraph", "Kismet", "KismetCompiler", "RenderCore", "AssetRegistry" });
	}
}
