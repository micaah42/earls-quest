// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class EarlsQuest : ModuleRules
{
	public EarlsQuest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "ProceduralMeshComponent",
            "HTTP",
            "Json",
            "JsonUtilities"
        });

        LoadOpen3D(Target);


		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}

    private string ThirdPartyPath
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/"));
        }
    }

    public bool LoadOpen3D(ReadOnlyTargetRules Target)
    {
        // only set up for Win64
        bool isLibrarySupported = false;

        // Create OpenCV Path 
        string Open3DPath = Path.Combine(ThirdPartyPath, "Open3D");

        // Get Library Path 
        string LibPath = "";
        bool isdebug = Target.Configuration == UnrealTargetConfiguration.Debug;
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            LibPath = Path.Combine(Open3DPath, "Libraries", "Win64");
            isLibrarySupported = true;
        }
        else
        {
            string Err = string.Format("{0} dedicated server is made to depend on {1}. We want to avoid this, please correct module dependencies.", Target.Platform.ToString(), this.ToString());
            System.Console.WriteLine(Err);
        }

        if (isLibrarySupported)
        {
            //Add Include path 
            PublicIncludePaths.AddRange(new string[] { 
                Path.Combine(Open3DPath, "includes"),
                Path.Combine(Open3DPath, "includes", "open3d", "3rdparty")
            });
            //Add Static Libraries
            PublicAdditionalLibraries.Add(Path.Combine(LibPath, "Open3D.lib"));
            //Add Dynamic Libraries
            PublicDelayLoadDLLs.Add("Open3D.dll");
        }
        PublicDefinitions.Add(string.Format("WITH_OPEN3D_BINDING={0}", isLibrarySupported ? 1 : 0));
        return isLibrarySupported;
    }
}
