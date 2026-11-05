[WorkbenchPluginAttribute(name: "EBT: Send To Blender", wbModules: { "ResourceManager" }, resourceTypes: { "fbx", "xob", "et" }, awesomeFontCode: 0xf1d8)]
class SendToBlenderPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	[Attribute("true", UIWidgets.CheckBox, "Import the mesh with the Import Settings of gamemats and layer presets from engine, that were overwritten by the user. This will overwrite the previous FBX settings.")]
	bool ImportColliderSettings;


	protected bool GetPathToResource(ResourceName resource, inout string absPath)
	{
		string path = resource.GetPath();
		Workbench.GetAbsolutePath(path, absPath);
		
		
		if (!FileIO.FileExists(absPath) || absPath == string.Empty)
			return false;
		
		if (resource.EndsWith(".et"))
			return true;
		
		if (absPath.EndsWith(".xob"))
			absPath.Replace(".xob", ".fbx");
		
		if (!FileIO.FileExists(absPath))
			return false;
		
		return true;
	}

	protected void OpenFBXWithBlender(string resource, string pathToBlenderExecutable)
	{
		string cmd = string.Format("\"%1\" --python-expr \"import bpy; bpy.ops.wm.read_homefile(use_empty=True); bpy.ops.ebt.import_fbx('EXEC_DEFAULT', filepath = r'%2')\"",
					 pathToBlenderExecutable, 
					 resource);

		Workbench.RunProcess(cmd);	
	}

	protected void OpenPrefabWithBlender(string resource, string pathToBlenderExecutable)
	{
		string cmd = string.Format("\"%1\" --python-expr \"import bpy; bpy.ops.wm.read_homefile(use_empty=True); bpy.ops.ebt.import_prefab('EXEC_DEFAULT', filepath = r'%2')\"", 
					 pathToBlenderExecutable,
					 resource);

		Workbench.RunProcess(cmd);
	}
	
	override void OnResourceContextMenu(notnull array<ResourceName> resources)
	{
		string pathToExecutable;
		string resourceAbsPath; 

		if (!EBTConfigPlugin.GetBlenderPathOrWarn(pathToExecutable))
			return;

		if (resources.IsEmpty())
		{
			Print("No selected resources", LogLevel.WARNING);
			return;
		}
		
		if(GetPathToResource(resources[0], resourceAbsPath))
		{	
			if(resources[0].EndsWith(".xob"))
				OpenFBXWithBlender(resourceAbsPath, pathToExecutable);
			else if(resources[0].EndsWith(".et"))
				OpenPrefabWithBlender(resourceAbsPath, pathToExecutable);
		}
		else
			Print("Resource couldn't be imported to blender because requested resource file is missing or is locked", LogLevel.WARNING);
	}
	
	

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Send")]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}
