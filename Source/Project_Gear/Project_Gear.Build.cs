using System.IO;
using UnrealBuildTool;

public class Project_Gear : ModuleRules
{
	public Project_Gear(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
        PublicIncludePaths.Add(ModuleDirectory);

        AddBullet();
	}

	private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
    }

    protected void AddBullet()
    {
        bool bDebug = Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame;
        bool bDevelopment = Target.Configuration == UnrealTargetConfiguration.Development;

        string BuildFolder = bDebug ? "Debug" :
            bDevelopment ? "RelWithDebInfo" : "Release";
        string BuildSuffix = bDebug ? "_Debug" :
            bDevelopment ? "_RelWithDebugInfo" : "";

        string LibrariesPath = Path.Combine(ThirdPartyPath, "bullet3", "lib", BuildFolder);
        PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "BulletCollision" + BuildSuffix + ".lib"));
        PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "BulletDynamics" + BuildSuffix + ".lib"));
        PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "LinearMath" + BuildSuffix + ".lib"));

        PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "bullet3", "src"));
        PublicDefinitions.Add("WITH_BULLET_BINDING=1");
    }
}