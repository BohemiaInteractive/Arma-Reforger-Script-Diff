class LoadedProjectsRequest : JsonApiStruct
{
	string path;

	void LoadedProjectsRequest()
	{
		RegV("path");
	}
}

class LoadedProjectsResponse : JsonApiStruct
{
	bool resourceInProject;

	void LoadedProjectsResponse()
	{
		RegV("resourceInProject");
	}
}


class LoadedProjects : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new LoadedProjectsRequest();
	}

	static bool InLoadedProjects(string path)
	{
		array<string> addons = new array<string>;
		//Get GUIDs of loaded addons
		GameProject.GetLoadedAddons(addons);
		string addonName, absPath;
		//Go throught the addons
		for (int i = 0; i < addons.Count(); i++)
		{
			//ID from the GUID
			addonName = GameProject.GetAddonID(addons[i]);
			//Find AbsPath to gproj
			Workbench.GetAbsolutePath("$" + addonName + ":", absPath, false);
			//TotalCommander uses small letters for disks so the path would be in lowercase as well
			absPath.ToLower();
			path.ToLower();
			if (absPath != "")
			{
				absPath = absPath.Substring(0, absPath.Length() - 1);
			}
			//If fbx path contains the gproj path
			if (path.Contains(absPath) && absPath != "")
			{
				return(true);
			}
			absPath.Replace("/","\\");
			if (path.Contains(absPath) && absPath != "")
			{
				return(true);
			}
		}
		return(false);

	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		LoadedProjectsRequest req = LoadedProjectsRequest.Cast(request);
		LoadedProjectsResponse response = new LoadedProjectsResponse();
		response.resourceInProject = InLoadedProjects(req.path);
		return response;
	}

}
