class PrefabImporterRequest : JsonApiStruct
{
	string absPath;
	int start;

	void PrefabImporterRequest()
	{
		RegV("absPath");
		RegV("start");
	}
}


class PrefabImporterResponse : JsonApiStruct
{
	string status;
	string message;
	// absolute fbx path
	ref array<string> fbx = new array<string>;
	// transformation
	ref array<string> coords = new array<string>;
	ref array<string> angles = new array<string>;
	ref array<string> scales = new array<string>;
	// socket names
	ref array<string> pivots = new array<string>;
	// scoket ids
	ref array<string> ids = new array<string>;
	// material name
	ref array<string> sourceMat = new array<string>;
	// path to rewritten emat
	ref array<string> ematPath = new array<string>;
	// index of a object in a fbx array on which the current object is parented
	ref array<int> parents = new array<int>;
	int stopped;
	int size;

	void PrefabImporterResponse()
	{
		RegV("status");
		RegV("message");
		RegV("fbx");
		RegV("coords");
		RegV("angles");
		RegV("scales");
		RegV("pivots");
		RegV("ids");
		RegV("sourceMat");
		RegV("ematPath");
		RegV("stopped");
		RegV("size");
		RegV("parents");
	}
}


class PrefabImporterUtils
{

	// in prefab it's impossible to get index of component
	// this function loops through all the components and returns "i" when the component is the same as the input
	int GetPrefabParamIndex(BaseContainer prefab, string name)
	{
		BaseContainerList srcList = prefab.GetObjectArray(EBTContainerFields.components);
		for (int i = 0; i < srcList.Count(); i++)
		{
			prefab = srcList.Get(i);
			if (prefab.GetClassName() == name)
			{
				return i;
			}
		}
		return -1;
	}

	
	// getting paramaters from the prefab file
	void GetPrefabParams(IEntitySource prefab, PrefabImporterResponse response)
	{
		BaseContainer empty;
		BaseContainer ancestorSource = prefab.GetAncestor();
		// edge-case for null prefab
		if (!ancestorSource)
		{
			return;
		}
		ResourceName ancestor = ancestorSource.GetResourceName();
		IEntitySource parent = prefab.GetParent();
		string pivotID = "";
		


		// get hierarchy
		int hierarchy = GetPrefabParamIndex(prefab, EBTContainerFields.hierarchy);
		if (hierarchy != -1)
		{
			// set pivot/socket ID and Name 
			BaseContainerList srcList = prefab.GetObjectArray(EBTContainerFields.components);
			BaseContainer prefabSocket = srcList.Get(hierarchy);
			ResourceName id = prefabSocket.GetResourceName();
			prefabSocket.Get(EBTContainerFields.pivot, pivotID);
			pivotID.ToLower();
			response.ids.Insert(prefab.GetID().ToString());
			response.pivots.Insert(pivotID);
		}
		else
		{
			// if not fill with garbage
			response.pivots.Insert("xxx");
		}
		

		//<-----------------Getting params----------------->\\
		string coords;
		string scale;

		// getting coords in local space
		if (prefab.IsVariableSet(EBTContainerFields.coords))
		{
			prefab.Get(EBTContainerFields.coords, coords);
		}
		else
		{
			coords = "0 0 0";
		}
		// getting scale
		if (prefab.IsVariableSet(EBTContainerFields.scale))
		{
			prefab.Get(EBTContainerFields.scale, scale);
		}
		else
		{
			scale = "1";
		}
		// getting angles as one string (EBT formatting)
		string angles = "0 0 0";
		// getting coords in local space
		if (prefab.IsVariableSet(EBTContainerFields.angles))
		{
			prefab.Get(EBTContainerFields.angles, angles);
		}


		// check if this fbx is parented to something in the prefab
		if (parent != empty)
		{
			int parentObjectID = GetPrefabParamIndex(parent, EBTContainerFields.meshObject);
			if (parentObjectID != -1)
			{
				// get parentFBX and check if the FBX already was processed
				string parentFbx = GetFBXPath(parent, parentObjectID, EBTContainerFields.object, response, false);
				for (int i = response.fbx.Count() - 1; i > 0; i--)
				{
					// if it was processed then we know its index and we add the index to this prefabs "parent" array since this prefab is parented to it
					if (response.fbx.Count() > 1 && parentFbx == response.fbx[i])
					{
						response.parents.Insert(i);
						break;
					}
				}
			}
		}

		// insert data
		response.coords.Insert(coords);
		response.scales.Insert(scale);
		response.angles.Insert(angles);

		// check if fbx paths is the same count as coords
		// if not add -1 as a parent since that means its the heighest prefab with no parents
		if (response.parents.Count() != response.coords.Count())
		{
			response.parents.Insert(-1);
		}

		// now get the absolute fbx path of the prefab we got parameters from
		int meshObjectID = GetPrefabParamIndex(prefab, EBTContainerFields.meshObject);
		
		// only if the meshObject exists
		// and read its ancestor
		if (meshObjectID != -1)
		{
			string fbxPath = GetFBXPath(prefab, meshObjectID, EBTContainerFields.object, response, true);
			// If filepath does not contain FBX, do not send it
			if (fbxPath.Contains(".fbx"))
			{
				response.fbx.Insert(fbxPath);
				ReadAncestor(ancestorSource, response, true);
			}
		}
		
		// if not then add a collection
		else
		{
			response.fbx.Insert("Collection-" + FilePath.StripExtension(FilePath.StripPath(prefab.GetResourceName())));
			ReadAncestor(ancestorSource, response, true);
		}
		return;
	}


	string GetFBXPath(IEntitySource prefab, int meshObjectID, string meshVarName, PrefabImporterResponse response, bool getEmat)
	{
		// get meshObjectID from the prefab file
		ResourceName absPath, output, relXob;
		array<string> sourceMat = new array<string>;
		Workbench.GetAbsolutePath(prefab.GetResourceName().GetPath(), absPath);
		BaseContainerList srcList = prefab.GetObjectArray(EBTContainerFields.components);
		IEntitySource prefabMesh = srcList.Get(meshObjectID);
		// if prefab has its own emat
		if (getEmat)
		{
			GetPrefabEmat(prefabMesh, response);
		}
		// get the fbx absolute path
		prefabMesh.Get(meshVarName, relXob);
		Workbench.GetAbsolutePath(relXob.GetPath(), output);
		output.Replace("xob","fbx");
		if (!FileIO.FileExists(output)){
			output = "";
		}
		return output;
	}

	// get materials that are overitten in prefab file
	void GetPrefabEmat(IEntitySource prefab, PrefabImporterResponse response)
	{
		ResourceName ematPath, absPathEmat;
		string matName;
		array<string> sourceMat = new array<string>;
		prefab.Get("Materials", ematPath);
		if (ematPath != "")
		{
			// format the material output
			ematPath.Split(";", sourceMat, false);
			for (int i = 0; i < sourceMat.Count(); i++)
			{
				//Material name
				matName = sourceMat[i].Substring(0, sourceMat[i].IndexOf(","));
				//Relative Path
				ematPath = sourceMat[i].Substring(sourceMat[i].IndexOf("{"), sourceMat[i].IndexOf(".emat") + 5 - sourceMat[i].IndexOf("{"));
				//Absolute Path
				Workbench.GetAbsolutePath(ematPath.GetPath(), absPathEmat);

				// insert data
				response.sourceMat.Insert(response.fbx.Count().ToString() + "|" + matName);
				response.ematPath.Insert(ematPath);
			}
		}
	}



	void ReadAncestor(IEntitySource prefab, PrefabImporterResponse response, bool getMeshObject)
	{

		// check if Ancestor of main prefab is null if yes then its a BasePrefab and we can return
		BaseContainer empty;
		BaseContainer ancestorMain = prefab.GetAncestor();
		if (ancestorMain == empty)
		{
			return;
		}

		// if not get its child
		ReadChild(prefab, response);
		// and then his ancestor
		ReadAncestor(ancestorMain, response, getMeshObject);
		return;
	}


	// get child params if the child is one of the predefined entity types
	void ReadChild(IEntitySource prefab, PrefabImporterResponse response)
	{
		for (int i = 0; i < prefab.GetNumChildren(); i++)
		{
			IEntitySource child = prefab.GetChild(i).ToEntitySource();
			if(EBTContainerFields.supportedTypes.Contains(child.GetClassName()))
			{
				GetPrefabParams(child, response);
			}
		}
	}

	void ReadFirst(ResourceName path, PrefabImporterResponse response)
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		//Get MetaFile from AbsPath of prefab (.et)
		MetaFile meta = resourceManager.GetMetaFile(path);
		Resource resource = Resource.Load(meta.GetResourceID());
		BaseContainer prefab = resource.GetResource().ToEntitySource();
		
		BaseContainer ancestorPrefab = prefab.GetAncestor();
		if(!ancestorPrefab)
		{
			return;
		}

		// get its paramas and read the child
		GetPrefabParams(prefab, response);
		ReadChild(prefab, response);
	}

	// JSONStructAPI has limitation what can be sent through
	// removing prefabs that were already sent or will cause the overload
	int RemoveOverloadedPrefabs(PrefabImporterResponse response, int startIndex)
	{
		const int DATA_CAP = 60000;
		// remove already sent prefabs
		for (int i = 0; i < startIndex; i++)
		{
			response.fbx.RemoveOrdered(0);
			response.scales.RemoveOrdered(0);
			response.angles.RemoveOrdered(0);
			response.coords.RemoveOrdered(0);
			response.pivots.RemoveOrdered(0);
			response.parents.RemoveOrdered(0);
		}

		// remove all other from last that will overload JSON
		while (response.GetSizeOf() > DATA_CAP)
		{
			response.fbx.RemoveOrdered(response.fbx.Count()-1);
			response.scales.RemoveOrdered(response.scales.Count()-1);
			response.angles.RemoveOrdered(response.angles.Count()-1);
			response.coords.RemoveOrdered(response.coords.Count()-1);
			response.pivots.RemoveOrdered(response.pivots.Count()-1);
			response.parents.RemoveOrdered(response.parents.Count()-1);
		}
		int stopIndex = response.fbx.Count() + startIndex;
		return stopIndex;

	}
}

class PrefabImporter : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new PrefabImporterRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		PrefabImporterRequest req = PrefabImporterRequest.Cast(request);
		PrefabImporterResponse response = new PrefabImporterResponse();
		PrefabImporterUtils utils = new PrefabImporterUtils();
		
		
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(req.absPath);
		
		if (!meta)
		{
			response.status = "ERROR";
			response.message = string.Format("Prefab Resource with path \"%1\" is missing!", req.absPath);
			return response;
		}
		
		response.status = "OK";
				
		// get prefabs with params
		utils.ReadFirst(req.absPath, response);
		
		// the size of the data
		response.size = response.fbx.Count();
		
		// removing overloaded data
		response.stopped = utils.RemoveOverloadedPrefabs(response, req.start);
		return response;
	}
}
