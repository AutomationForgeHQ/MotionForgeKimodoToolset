using UnrealBuildTool;

public class MotionForgeKimodoToolset : ModuleRules
{
	public MotionForgeKimodoToolset(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"ToolsetRegistry",    // UToolsetDefinition and UAgentSkill are public base classes
				"MotionForgeKimodo",  // the capability this exposes, and the types in its signatures
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",         // IPluginManager, so GetToolsetVersion() reads the descriptor
			}
			);

		// As with MotionForgeToolset: no dependency on ModelContextProtocol. Tools register with
		// ToolsetRegistry and MCP picks them up from there.
	}
}
