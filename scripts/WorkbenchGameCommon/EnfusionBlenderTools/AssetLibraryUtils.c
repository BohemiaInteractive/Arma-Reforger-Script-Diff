class GetLoadedProjectsResponse : JsonApiStruct
{
	ref array<string> loadedProjects;

	void GetLoadedProjectsResponse(array<string> projects)
	{
		this.loadedProjects = projects;
	}
	
	override void OnPack()
	{
		StartArray("Loaded Projects");
		foreach (string item : loadedProjects)
		{
			ItemString(item);
		}
		EndArray();
	}
}

static array<string> GetLoadedProjectsArray()
{
	array<string> addons = new array<string>;
	array<string> result = new array<string>();
	//Get GUIDs of loaded addons
	GameProject.GetLoadedAddons(addons);
	string addonName, absPath;
	//Go throught the addons
	foreach (string addon : addons)
	{
		//ID from the GUID
		addonName = GameProject.GetAddonID(addon);
		//Find AbsPath to gproj
		if(addonName != "core")
		{
			Workbench.GetAbsolutePath("$" + addonName + ":", absPath, false);
			result.Insert(addonName);
		}
	}
	return result;
}

class GetLoadedProjects : NetApiHandler
{
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		return new GetLoadedProjectsResponse(GetLoadedProjectsArray());
	}
}

//-------------------------------------------------------

class LocateProjectRequest : JsonApiStruct
{
	string path;		

	void LocateProjectRequest()
	{
		RegV("path");
	}
}

class LocateProjectResponse : JsonApiStruct
{
	private string status;
	private string message;
	private string projectName;
	private string projectPath;
	
	void LocateProjectResponse(string _status, string _message, string _projectName, string _projectPath)
	{
		status = _status;
		message = _message;
		projectName = _projectName;
		projectPath = _projectPath;
		RegAll();
	}
}

class LocateProject : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new LocateProjectRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		LocateProjectRequest req = LocateProjectRequest.Cast(request);
		
		array<string> loadedAddons = new array<string>;
		GameProject.GetLoadedAddons(loadedAddons);
		
		foreach (string addon : loadedAddons)
		{
			string addonName = GameProject.GetAddonID(addon);
			string absPath;
			
			Workbench.GetAbsolutePath("$" + addonName + ":", absPath, false);
			if (absPath != "" && req.path.Contains(absPath))
			{
				return new LocateProjectResponse("OK", "", addonName, absPath);
			}
		}
		
		return new LocateProjectResponse("ERROR", string.Format("Could not find active project in path \"%1\". Please select a different path.", req.path), "", "");
	}
}

//-------------------------------------------------------

class LocatePrefabsFromPathRequest : JsonApiStruct
{
	string selectedProject;
	string projectPath;
	string selectedFolder;

	void LocatePrefabsFromPathRequest()
	{
		RegV("selectedProject");
		RegV("projectPath");
		RegV("selectedFolder");
	}
}

class LocatePrefabsFromPathResponse : JsonApiStruct
{
	private string status;
	private string message;
	private ref array<string> prefabsPaths;
	
	void LocatePrefabsFromPathResponse(string _status, string _message, array<string> paths)
	{
		status = _status;
		message = _message;
		prefabsPaths = paths;
		
		RegV("status");
		RegV("message");
		RegV("prefabsPaths");
	}
}

class LocatePrefabsFromPath : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new LocatePrefabsFromPathRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		LocatePrefabsFromPathRequest req = LocatePrefabsFromPathRequest.Cast(request);
		array<string> paths = {};
		
		// get absolute path to the project
		string projectAbsPath;
		Workbench.GetAbsolutePath("$" + req.selectedProject + ":", projectAbsPath, false);
		bool folderExists = false;
		if (req.selectedFolder.Contains(req.projectPath))
		{
			string absPath = req.selectedFolder;
			absPath.Replace("\\", "/");
			FileIO.FindFiles(paths.Insert, absPath, ".et");
			if (paths.Count() == 0)
			{
				string errorMessage = "Could not find any Prefabs! Does " + req.selectedFolder + " folder exist in your project? Does " + req.selectedFolder + " folder contain any prefabs?";
				return LocatePrefabsFromPathResponse("ERROR", errorMessage, null);
			}
		}
		else
		{
			string errorMessage = "Could not find any Prefabs! Does " + req.selectedFolder + " folder exist in your project? Does " + req.selectedFolder + " folder contain any prefabs?";
			return LocatePrefabsFromPathResponse("ERROR", errorMessage, null);
		}
		
		int messageApproxSize;
		
		foreach(string entry : paths)
		{
			messageApproxSize += entry.Length();
		}
		
		if (messageApproxSize > 500000)
		{
			string errorMessage = "Too many prefabs! (Approximate size " + (messageApproxSize/1000) + " kB, limit is 512 kB.) Please try importing a directory containing less prefabs.";
			return LocatePrefabsFromPathResponse("ERROR", errorMessage, null);
		}
		
		return LocatePrefabsFromPathResponse("OK", "", paths);
	}
}

//-------------------------------------------------------

class GetPrefabGUIDRequest : JsonApiStruct
{
	string prefabPath;

	void GetPrefabGUIDRequest()
	{
		RegV("prefabPath");
	}
}

class GetPrefabGUIDResponse : JsonApiStruct
{
	private string status;
	private string message;
	private string guid;
	
	void GetPrefabGUIDResponse(string _status, string _message, string _guid)
	{
		status = _status;
		guid = _guid;
		message = _message;
		
		RegV("status");
		RegV("message");
		RegV("guid");
	}
}

class GetPrefabGUID : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new GetPrefabGUIDRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetPrefabGUIDRequest req = GetPrefabGUIDRequest.Cast(request);
		
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		MetaFile meta = rm.GetMetaFile(req.prefabPath);
		
		// check if meta exists
		if (!meta)
		{
			return GetPrefabGUIDResponse("ERROR", "Meta file not found!", "");
		}

		// get data
		string name = meta.GetResourceID();
		string guid = name.Substring(name.IndexOf("{") + 1, name.IndexOf("}") - 1);
				
		return GetPrefabGUIDResponse("OK", "GUID found.", guid);
	}
}

//-------------------------------------------------------

class GetPathToAssetsFromGuidsRequest : JsonApiStruct
{
	ref array<string> guids;

	void GetPathToAssetsFromGuidsRequest()
	{
		RegV("guids");
	}
}

class GetPathToAssetsFromGuidsResponse : JsonApiStruct
{
	ref array<ResourceName> prefabsPaths;
	ref array<string> invalidGuids;

	void GetPathToAssetsFromGuidsResponse(array<ResourceName> paths, array<string> _invalidGuids)
	{
		prefabsPaths = paths;
		invalidGuids = _invalidGuids;
		
		RegV("prefabsPaths");
		RegV("invalidGuids");
	}
}

class GetPathToAssetsFromGuids : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new GetPathToAssetsFromGuidsRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetPathToAssetsFromGuidsRequest req = GetPathToAssetsFromGuidsRequest.Cast(request);
		array<ResourceName> paths = {};
		array<string> invalidGuids = {};
		
		foreach (string guid : req.guids)
		{
			ResourceName guidRes = "{" + guid + "}";
			ResourceName abs;
			
			if (Workbench.GetAbsolutePath(guidRes.GetPath(), abs, true))
			{
				if (abs.Contains(".et"))
				{
					paths.Insert(abs);
				}
				else
				{
					invalidGuids.Insert(guid);
				}
			}
		}
	
		return GetPathToAssetsFromGuidsResponse(paths, invalidGuids);
	}
}