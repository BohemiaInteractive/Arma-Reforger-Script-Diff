#ifdef WORKBENCH

static BaseContainer GetContainerFromList(string containerName, BaseContainerList containerList)
{
	for (int i = 0; i < containerList.Count(); i++)
	{
		if (containerList[i].GetClassName() == containerName)
			return containerList.Get(i);
	}
	return null;
}

static const ref array<string> st = {"SCR_DestructibleBuildingEntity", "GenericEntity", "Building", "GameEntity", "StaticModelEntity"};

FBXItem PopulateFBXItem(BaseContainer prefab, FBXItem slotOwner)
{
	if (!st.Contains(prefab.GetClassName()))
		return null;

	BaseContainerList components = prefab.GetObjectArray(EBTContainerFields.components);
	BaseContainer meshObject = GetContainerFromList(EBTContainerFields.meshObject, components);

	if (meshObject)
	{
		FBXItem result = new FBXItem();

		ResourceName object;
		meshObject.Get(EBTContainerFields.object, object);

		string absPath = "";
		Workbench.GetAbsolutePath(object.GetPath(), absPath, true);
		absPath.Replace(".xob", ".fbx");
		result.absPath = absPath;

		BaseContainerList materials = meshObject.GetObjectArray("Materials");

		if (materials)
		{
			for (int i = 0; i < materials.Count(); i++)
			{
				BaseContainer materialData = materials.Get(i);
				ResourceName sourceMaterial, assignedMaterial;

				materialData.Get("SourceMaterial", sourceMaterial);
				materialData.Get("AssignedMaterial", assignedMaterial);
				result.sourceToAssignedMaterial.Insert(sourceMaterial, assignedMaterial);
			}
		}

		BaseContainer hierarchyComponent = GetContainerFromList(EBTContainerFields.hierarchy, components);
		if (hierarchyComponent)
		{
			string pivotID;

			hierarchyComponent.Get(EBTContainerFields.pivot, pivotID);

			if (pivotID.Contains("glass"))
				return null;

			string checkForEntry = pivotID;
			checkForEntry.ToLower();

			if (pivotID && !checkForEntry.Contains("entry"))
			{
				if (slotOwner.slotToFbxChild.Contains(pivotID))
					slotOwner.slotToFbxChild.Get(pivotID).Insert(result);
				else
				{
					array<ref FBXItem> value = {result};
					slotOwner.slotToFbxChild.Insert(pivotID, value);
				}
			}
		}

		for (int i = 0; i < prefab.GetNumChildren(); i++)
		{
			PopulateFBXItem(prefab.GetChild(i), result);
		}

		return result;
	}

	return null;
}

class FBXItem
{
	string absPath;
	ref map<string, string> sourceToAssignedMaterial = new map<string, string>();
	ref map<string, ref array<ref FBXItem>> slotToFbxChild = new map<string, ref array<ref FBXItem>>();
}

class PrefabImportRequest : JsonApiStruct
{
	string etAbsPath;

	void PrefabImportRequest()
	{
		RegV("etAbsPath");
	}
}

class PrefabImportErrorResponse : JsonApiStruct
{
	string msg = "ERROR";

	void PrefabImportErrorResponse()
	{
		RegV("msg");
	}
}

class PrefabImportResponse : JsonApiStruct
{
	ref FBXItem exportItem;

	void PrefabImportResponse(FBXItem item)
	{
		exportItem = item;
	}

	void SerializeFBXItem(FBXItem item)
	{
		StartObject("fbx");
			StoreString("path", item.absPath);

			if (item.sourceToAssignedMaterial.Count() != 0)
			{
				StartObject("material_overrides");
				foreach (string sourceMaterial, string assignedMaterial : item.sourceToAssignedMaterial)
				{
					StoreString(sourceMaterial, assignedMaterial);
				}
				EndObject();
			}

			if (item.slotToFbxChild.Count() != 0)
			{
				StartObject("sockets");
				foreach (string slot, ref array<ref FBXItem> items : item.slotToFbxChild)
				{
					StartObject(slot);
					foreach (ref FBXItem i : items)
					{
						SerializeFBXItem(i);
					}
					EndObject();
				}
				EndObject();
			}

		EndObject();
	}

	override void OnPack()
	{
		SerializeFBXItem(exportItem);
	}
}

class PrefabImporterBake : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new PrefabImportRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		PrefabImportRequest testRequest = PrefabImportRequest.Cast(request);
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(testRequest.etAbsPath);

		Resource resource = Resource.Load(meta.GetResourceID());
		BaseContainer prefab = resource.GetResource().ToBaseContainer();
		FBXItem fbx = PopulateFBXItem(prefab, null);

		if (!fbx)
		{
			PrefabImportErrorResponse response = new PrefabImportErrorResponse();
			return response;
		}

		PrefabImportResponse response = new PrefabImportResponse(fbx);
		return response;
	}

	static bool GetBaseFBX(ResourceName resourceName, out string absPath)
	{
		Resource resource = Resource.Load(resourceName);
		BaseContainer prefab = resource.GetResource().ToBaseContainer();

		BaseContainerList components = prefab.GetObjectArray(EBTContainerFields.components);
		BaseContainer meshObject = GetContainerFromList(EBTContainerFields.meshObject, components);
		
		if(!meshObject)
			return false;

		ResourceName object;
		meshObject.Get(EBTContainerFields.object, object);

		string path;
		Workbench.GetAbsolutePath(object.GetPath(), path, true);
		path.Replace(".xob", ".fbx");
		absPath = path;
		
		return true;
	}
}

#endif
