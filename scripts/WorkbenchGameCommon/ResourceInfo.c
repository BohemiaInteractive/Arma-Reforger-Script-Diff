/*
	EXPORT
*/

// Copied over from TextureImportTool
bool IsImage(string className)
{
	return
		className == "PNGResourceClass" ||
		className == "DDSResourceClass" ||
		className == "TGAResourceClass" ||
		className == "TIFFResourceClass" ||
		className == "PNGResourceClass" ||
		className == "HDRResourceClass" ||
		className == "JPGResourceClass";
}

MetaFile GetMetaFileFromString(string str)
{
	
	ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
	
	// Try Absolute Path
	if (FilePath.IsAbsolutePath(str))
	{
		if (!FileIO.FileExists(str))
		{
			return null;
		}
		
		return resourceManager.GetMetaFile(str);
	}
	
	// Try Relative Path (relative to current project)
	auto meta = resourceManager.GetMetaFile(str);
	
	if (meta != null) {
		return meta;
	}
		
	// Try ResourceName, will return null in case of invalid resource name.
	ResourceName resourceName = str;
	
	string resourcePath = resourceName.GetPath();
	
	return resourceManager.GetMetaFile(resourcePath);
}
	
	


class ContainerJSONSerializer
{	
	static string ignoredFields[8] = {"userScript", "constructor", "destructor", "editorData", "RplLoad", "RplSave", "Preload", "OnTransformResetImpl"};
	
	//----------------------------------------------------------------------------------------------
	static void WriteVars(JsonApiStruct jsonStruct, BaseContainer container, bool expand = false)
	{
		
		int childrenCount = container.GetNumChildren();
		if (childrenCount > 0)
			jsonStruct.StoreInteger("ChildrenCount", childrenCount); // use data.get("ChildrenCount", 0) on the Python side
		
		for (int i = 0; i < container.GetNumVars(); i++)
		{
			string varName = container.GetVarName(i);
			/* 
				Effort to reduce the size of the JSON as we often exced the limit of 524320 bytes when trying to send information about the Prefab.
				TODO: define fields we want for each resource type instead of ignore list. Last option would be to send data in chunks.
			*/
			if (varName == "" || varName.StartsWith("_WB") || varName.StartsWith("EOn"))  continue;
				
			bool ignore;
			foreach (auto field : ignoredFields)
			{
				if (varName == field)
				{
					ignore = true;
					break;
				}
			}
			
			if (ignore) continue;
			
			
			DataVarType varType = container.GetDataVarType(i);
			
			switch (varType)
			{
				case DataVarType.COLOR:
				{
					Color value;
					container.Get(varName, value);
					jsonStruct.StartArray(varName);
						jsonStruct.ItemFloat(value.R());
						jsonStruct.ItemFloat(value.G());
						jsonStruct.ItemFloat(value.B());
						jsonStruct.ItemFloat(value.A());
					jsonStruct.EndArray();
					break;
				}
				case DataVarType.STRING_ARRAY:
				{
					array<string> value;
					container.Get(varName, value);
					jsonStruct.StartArray(varName);
					foreach (string item : value)
					{
						jsonStruct.ItemString(item);
					}
					jsonStruct.EndArray();
	
					break;
				}
				case DataVarType.RESOURCE_NAME:
				{
					ResourceName value;
					container.Get(varName, value);
	
					if (value != "")
					{
						if (expand)
						{
							ResourceInfo resourceInfo = ResourceInfo.CreateFromString(value, expand);
							 jsonStruct.StoreObject(varName, resourceInfo);
							 break;
						}
					}
	
					jsonStruct.StoreString(varName, value);
					break;
				}
				case DataVarType.RESOURCE_NAME_ARRAY:
				{
					array<ResourceName> value;
	
					container.Get(varName, value);
					jsonStruct.StartArray(varName);
					foreach (ResourceName rName : value)
					{
						jsonStruct.ItemString(rName);
					}
					jsonStruct.EndArray();
					break;
				}
				case DataVarType.STRING:
				{
					string value;
					container.Get(varName, value);
	
					jsonStruct.StoreString(varName, value);
					break;
				}
				case DataVarType.TEXTURE:
				{
					string values;
					container.Get(varName, values);
	
					string value = "";
	
					if (values != "" && values != " 0")
					{
						array<string> spltValue = new array<string>();
						values.Split(" ", spltValue, true);
	
						value = spltValue[0];
						if (expand)
						{
							ResourceInfo resourceInfo = ResourceInfo.CreateFromString(value, expand);
							jsonStruct.StoreObject(varName, resourceInfo);
							break;
						}
					}
					jsonStruct.StoreString(varName, value);
					break;
				}
				case DataVarType.SCALAR:
				{
					float value;
					container.Get(varName, value);
					jsonStruct.StoreFloat(varName, value);
					break;
				}
				case DataVarType.SCALAR_ARRAY:
				{
					array<float> value;
					container.Get(varName, value);
					jsonStruct.StartArray(varName);
					foreach (float item : value)
					{
						jsonStruct.ItemFloat(item);
					}
					jsonStruct.EndArray();
					break;
				}
				case DataVarType.BOOLEAN:
				{
					bool value;
					container.Get(varName, value);
					jsonStruct.StoreBoolean(varName, value);
					break;
				}
				case DataVarType.INTEGER:
				{
					int value;
					container.Get(varName, value);
					
					jsonStruct.StoreInteger(varName, value);
					break;
				}
				case DataVarType.VECTOR3:
				{
					vector value;
					container.Get(varName, value);
					
					jsonStruct.StartArray(varName);
						jsonStruct.ItemFloat(value[0]);
						jsonStruct.ItemFloat(value[1]);
						jsonStruct.ItemFloat(value[2]);
					jsonStruct.EndArray();
					break;
				}
				case DataVarType.OBJECT:
				{
					BaseContainer object = container.GetObject(varName);
					if (object)
					{
						jsonStruct.StoreObject(varName, new GenericJSONContainer(object, expand));
						break;
					}
					
					jsonStruct.StartObject(varName);
					jsonStruct.EndObject();
					break;
				}
				case DataVarType.OBJECT_ARRAY:
				{
					BaseContainerList objects = container.GetObjectArray(varName);
	
					jsonStruct.StartArray(varName);
					for (int objIdx = 0; objIdx < objects.Count(); objIdx++)
					{
						jsonStruct.ItemObject(new GenericJSONContainer(objects.Get(objIdx), expand));
					}
					jsonStruct.EndArray();
					break;
				}
				default:
				{
					//Print(string.Format("Unsupported DataVarType: \"%1\"", typename.EnumToString(DataVarType, varType)));
				}
			}
		}
		
		/*
			Reduce size of JSON
		*/
//		int numChildren = container.GetNumChildren();
//		
//		if(numChildren)
//		{
//			jsonStruct.StartArray("Children");
//			for(int i = 0; i < numChildren; i++)
//			{
//				BaseContainer childContainer = container.GetChild(i);
//				
//				if(childContainer)
//				{
//					jsonStruct.ItemObject(new GenericJSONContainer(childContainer, true, expand));
//				}
//			}
//			jsonStruct.EndArray();
//		}
		
	}
}

enum InfoStatus
 {
 	OK,
 	ERROR,
 }

/*!
Dummy class to be used with StoreObject and ItemObject that expect a JsonApiStruct as parameter
*/
class GenericJSONContainer : JsonApiStruct
{
	BaseContainer m_Container;
	bool m_bExpandResource = false;

	//----------------------------------------------------------------------------------------------
	void GenericJSONContainer(BaseContainer container, bool expand = false)
	{
		m_Container = container;
		
		m_bExpandResource = expand;
	}

	//----------------------------------------------------------------------------------------------
	override void OnPack()
	{
		string objectName = string.Format("%1", m_Container.GetClassName());
		
		StoreString("ClassName", objectName);
		StoreString("Name", m_Container.GetName());
		ContainerJSONSerializer.WriteVars(this, m_Container, m_bExpandResource);
	}
}

class TextureResourceInfo : JsonApiStruct
{
	private string TextureSourcePath;
 	private string PostFix;
 	private string ColorSpace;
 	private string Conversion;
 	private bool ContainsMips;
	
	//----------------------------------------------------------------------------------------------
 	void TextureResourceInfo(string textureSourcePath, string postFix, string colorSpace, string conversion, bool containMips)
 	{
 		TextureSourcePath = textureSourcePath;
 		PostFix = postFix;
 		ColorSpace = colorSpace;
 		Conversion = conversion;
 		ContainsMips = containMips;
 		RegAll();
 	}
	
 	protected static ref TextureTypes Types = new TextureTypes();
	
	//----------------------------------------------------------------------------------------------
 	static string GetPostFix(string resourcePath)
 	{
 		return Types.FindTextureType(resourcePath).m_PostFix;
 	}
 }

class MaterialResourceInfo : JsonApiStruct
{
	private ResourceName m_ResourceName;
	private bool m_bExpandResource = false;
	private array<string> m_Fields;
	
	//----------------------------------------------------------------------------------------------
	void MaterialResourceInfo(ResourceName resourceName, bool expand = false)
	{
		m_ResourceName = resourceName;
		m_bExpandResource = expand;
	}

	//----------------------------------------------------------------------------------------------
	override void OnPack()
	{
		Resource resource = Resource.Load(m_ResourceName);
	
		BaseContainer basecontainer = resource.GetResource().ToBaseContainer();
		StoreString("ShaderClass", basecontainer.GetClassName());
		
		ContainerJSONSerializer.WriteVars(this, basecontainer, m_bExpandResource);
	}
}

class XOBResourceInfo : JsonApiStruct
{
	private string m_FBXSourcePath;
	private BaseContainer m_ConfigurationContainer;
	private bool m_bExpandResource = false;

	//----------------------------------------------------------------------------------------------
	void XOBResourceInfo(notnull BaseContainer configurationContainer, string fbxSourcePath, bool expand = false)
	{
		m_FBXSourcePath = fbxSourcePath;
		m_ConfigurationContainer = configurationContainer;
		m_bExpandResource = expand;
	}

	//----------------------------------------------------------------------------------------------
	override void OnPack()
	{
		StoreString("FBXSourcePath", m_FBXSourcePath);
		ContainerJSONSerializer.WriteVars(this, m_ConfigurationContainer, m_bExpandResource);
	}
}


class GenericResourceInfo : JsonApiStruct
{
	private ResourceName m_ResourceName;
	private bool m_bExpandResource = false;

	//----------------------------------------------------------------------------------------------
	void GenericResourceInfo(ResourceName resourceName, bool expand = false)
	{
		m_ResourceName = resourceName;
		m_bExpandResource = expand;
	}

	//----------------------------------------------------------------------------------------------
	override void OnPack()
	{
		Resource resource = Resource.Load(m_ResourceName);
	
		BaseContainer basecontainer = resource.GetResource().ToBaseContainer();
		StoreString("ClassName", basecontainer.GetClassName());
		
		ContainerJSONSerializer.WriteVars(this, basecontainer, m_bExpandResource);
	}
}


class ResourceInfo : JsonApiStruct
{
	private InfoStatus status;
	private string message;
	private string resourceName;
	private string absolutePath; 
	private ref JsonApiStruct data;

	//----------------------------------------------------------------------------------------------
	void ResourceInfo(InfoStatus _status, ResourceName _resourceName, string _absolute_path, string _message, JsonApiStruct _data)
	{
		status = _status;
		resourceName = _resourceName;
		message = _message;
		data = _data;
		absolutePath = _absolute_path;
	
		RegAll();
		if (data == null)
		UnregV("data");
	}

	//----------------------------------------------------------------------------------------------
	private static string CreateSourceFileNotFoundmessage(ResourceName _resourceName)
	{
		return string.Format("Source file not found for resource: %1", _resourceName);
	}

	//----------------------------------------------------------------------------------------------
	static ResourceInfo CreateFromString(string path, bool expand = false)
	{
		auto meta = GetMetaFileFromString(path);
	
		if (!meta)
		{
			return ResourceInfo(
			InfoStatus.ERROR, 
			string.Empty,
			string.Empty,
			string.Format("Resource not found: %1", path), 
			null);
		}
		
		return ResourceInfo.CreateFromMetaFile(meta, expand);
	}
	
	static ResourceInfo CreateFromMetaFile(MetaFile meta, bool expand = false)
	{
		BaseContainer configurationContainer = meta.GetObjectArray("Configurations")[0];
		
		auto _resourceName = meta.GetResourceID();
		string resourcePath = _resourceName.GetPath();
		
		if (!configurationContainer)
		{
			return ResourceInfo(
				InfoStatus.ERROR,
				_resourceName,
				string.Empty,
				string.Format("Invalid configuration container for resource: %1", _resourceName), 
				null);
		}
		
			string className = configurationContainer.GetClassName();
		
		if (className == "FBXResourceClass")
		{
			string fbxSourcePath;
			if (!Workbench.GetAbsolutePath(meta.GetSourceFilePath(), fbxSourcePath, true))
	 		{
	 			return ResourceInfo(
					InfoStatus.ERROR,
					_resourceName,
					string.Empty,
					CreateSourceFileNotFoundmessage(_resourceName),
					null);
	 		}
			
	 		return ResourceInfo(InfoStatus.OK, _resourceName, fbxSourcePath, string.Empty, new XOBResourceInfo(configurationContainer, fbxSourcePath, expand));
		}
		
		if (className == "EMATResourceClass")
		{
			Resource resource = Resource.Load(_resourceName);
			
	 		if (!resource.IsValid())
	 		{
	 			return ResourceInfo(
					InfoStatus.ERROR, 
					_resourceName,
					string.Empty,
					string.Format("Resource could not be loaded: %1", _resourceName),
					null);
			}
			
			string file_path = resource.GetResource().GetResourceName().GetPath();
			string absolute_path;
			
			bool result = Workbench.GetAbsolutePath(file_path, absolute_path, true);
			
			if (!result){ 
				absolute_path = string.Empty;
			}
			
			
	 		return ResourceInfo(InfoStatus.OK, _resourceName, absolute_path, string.Empty, new MaterialResourceInfo(_resourceName, expand));
		}
		
		
		if(className == "EntityTemplateResourceClass" || className == "GameMaterialResourceClass")
		{
			Resource resource = Resource.Load(_resourceName);
			
	 		if (!resource.IsValid())
	 		{
	 			return ResourceInfo(
					InfoStatus.ERROR, 
					_resourceName,
					string.Empty,
					string.Format("Resource could not be loaded: %1", _resourceName),
					null);
			}
			
			if (className == "GameMaterialResourceClass") //Hack to prevent recursion
				expand = false;
			
			string file_path = resource.GetResource().GetResourceName().GetPath();
			string absolute_path;
			
			bool result = Workbench.GetAbsolutePath(file_path, absolute_path, true);
			
			if (!result){ 
				absolute_path = string.Empty;
			}
			
	 		return ResourceInfo(InfoStatus.OK, _resourceName, absolute_path, "", new GenericResourceInfo(_resourceName, expand));
		}
		
		if(className == "CONFResourceClass")
		{			
			Resource resource = BaseContainerTools.LoadContainer(_resourceName);
			
			if (resource == null || !resource.IsValid())	
	 		{
	 			return ResourceInfo(
					InfoStatus.ERROR, 
					_resourceName,
					string.Empty,
					string.Format("Resource could not be loaded: %1", _resourceName),
					null);
			}
			
			string file_path = resource.GetResource().GetResourceName().GetPath();
			string absolute_path;
			
			bool result = Workbench.GetAbsolutePath(file_path, absolute_path, true);
			
			if (!result){ 
				absolute_path = string.Empty;
			}
			
	 		return ResourceInfo(InfoStatus.OK, _resourceName, absolute_path, "", new GenericResourceInfo(_resourceName, expand));
		}
		
		if (IsImage(className))
		{
			string textureSourcePath;
	 		if (!Workbench.GetAbsolutePath(meta.GetSourceFilePath(), textureSourcePath, true))
	 		{
	 			return ResourceInfo(
					InfoStatus.ERROR, 
					_resourceName,
					string.Empty,
					CreateSourceFileNotFoundmessage(_resourceName),
					null);
	 		}
			
	 		string postFix = TextureResourceInfo.GetPostFix(resourcePath);
	 		int colorSpaceEnumValue;
	 		int conversionEnumValue;
			int containsMips;
			
	 		configurationContainer.Get("ColorSpace", colorSpaceEnumValue);
	 		configurationContainer.Get("Conversion", conversionEnumValue);
			configurationContainer.Get("ContainsMips", containsMips);
			
			
			string colorSpace = typename.EnumToString(MetaEddsColorSpaceConversion, colorSpaceEnumValue);
			string conversion = typename.EnumToString(MetaEddsConversion, conversionEnumValue);
			
	 		TextureResourceInfo textureProperty = new TextureResourceInfo(textureSourcePath, postFix, colorSpace, conversion, containsMips);
			
	 		return ResourceInfo(InfoStatus.OK, _resourceName, textureSourcePath, string.Empty, textureProperty);
		}
		
		if (className == "TXAResourceClass")
		{
			string absolute_path;
			bool result = Workbench.GetAbsolutePath(resourcePath, absolute_path, true);
			
			if (!result){ 
				absolute_path = string.Empty;
			}
			
			return ResourceInfo(InfoStatus.OK, _resourceName, absolute_path, "", null);
		}
		
		return ResourceInfo(
			InfoStatus.ERROR, 
			_resourceName,
			string.Empty,
			string.Format("Unsuported resource type: %1 for resource: %2", className, _resourceName),
			null); 
	}
}

class GetResourceInfoRequest : JsonApiStruct
{
	string path;
	bool expandResource;
	
	//----------------------------------------------------------------------------------------------
	void GetResourceInfoRequest()
	{
		RegAll();
	}
}

class GetResourceInfo : NetApiHandler
{
	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetRequest()
	{
		return new GetResourceInfoRequest();
	}

	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetResourceInfoRequest req = GetResourceInfoRequest.Cast(request);
		
		return ResourceInfo.CreateFromString(req.path, req.expandResource);
	}
}


class GetResourceValidityRequest : JsonApiStruct
{
    ref array<ResourceName> resources = new array<ResourceName>;

    void GetResourceValidityRequest()
    {
        RegV("resources");
    }
}

class GetResourceValidityResponse : JsonApiStruct
{
	ref array<bool> validity = new array<bool>;

	void GetResourceValidityResponse()
	{
		RegV("validity");
	}
}

class GetResourceValidity: NetApiHandler
{
    override JsonApiStruct GetRequest()
    {
        return new GetResourceValidityRequest();
    }

    override JsonApiStruct GetResponse(JsonApiStruct request)
    {
		GetResourceValidityRequest req = GetResourceValidityRequest.Cast(request);
		GetResourceValidityResponse response = new GetResourceValidityResponse;

		foreach (string resourceName : req.resources)
		{
			Resource resource = BaseContainerTools.LoadContainer(resourceName);
			
			if (resource == null || !resource.IsValid()) response.validity.Insert(false);
			else response.validity.Insert(true);
		}
		
		return response;
	}
}


/*
	PREFAB CHILD
*/

class GetPrefabChildRequest: JsonApiStruct
{
	ResourceName resourceName; //prefab resource name
	ref array<int> childIdx = new array<int>;
	bool expandResource;
	
	//----------------------------------------------------------------------------------------------
	void GetPrefabChildRequest()
	{
		RegAll();
	}
}

class GetPrefabChildInfo: NetApiHandler
{
	private ref Resource mResource;
	
	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetRequest()
	{
		return new GetPrefabChildRequest();
	}

	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetPrefabChildRequest req = GetPrefabChildRequest.Cast(request);
		
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(req.resourceName.GetPath());
		
		if (!meta)
		{
			return ResourceInfo(
			InfoStatus.ERROR, 
			req.resourceName,
			string.Empty,
			string.Format("Resource not found: %1", req.resourceName), 
			null);
		}
		
		mResource = Resource.Load(req.resourceName);
		BaseContainer baseContainer = mResource.GetResource().ToBaseContainer();
		
		BaseContainer childContainer = baseContainer;
		
		for (int i = 0; i < req.childIdx.Count(); i++)
		{
			childContainer = childContainer.GetChild(req.childIdx[i]);
		}
		
		
		if (!childContainer)
		{
			return ResourceInfo(
			InfoStatus.ERROR, 
			req.resourceName,
			string.Empty,
			string.Format("Prefab doesn't have child with index: %1", req.childIdx), 
			null);
		}
		
		GenericJSONContainer data = new GenericJSONContainer(childContainer, req.expandResource);
		
		return ResourceInfo(InfoStatus.OK, "", string.Empty, string.Empty, data);
	}
}


enum ExportStatus
{
	OK,
	ERROR
}

class ExportResult: JsonApiStruct
{
	private ExportStatus status;
	private string resourcePath;
	private string message;
	
	void ExportResult(ExportStatus _status, string _resourcePath, string _message = "")
	{
		status = _status;
		resourcePath = _resourcePath;
		message = _message;
		RegAll();
	}
}

/*
	GAME MATERIALS
*/


class GetGameMaterialsResponse : JsonApiStruct
{
	ref array<ResourceName> data = new array<ResourceName>;
	
	//----------------------------------------------------------------------------------------------
	void GetGameMaterialsResponse()
	{
		RegAll();
	}
}

class GetGameMaterials: NetApiHandler
{
	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetRequest()
	{
		return null;
	}

	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		map<string, string> result = new map<string, string>();		
		
		GetGameMaterialsResponse response = new GetGameMaterialsResponse;
		
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = {"gamemat"};
		
		ResourceDatabase.SearchResources(filter, response.data.Insert); 
		
		return response;
	}
}

class GetMaterialsByClassRequest : JsonApiStruct
{
    string matClass;

    void GetMaterialsByClassRequest()
    {
        RegV("matClass");
    }
}

class GetMaterialsByClassResponse : JsonApiStruct
{
	InfoStatus status;
	string message;
	ref array<ResourceName> data = new array<ResourceName>;

	void GetMaterialsByClassResponse()
	{
		RegAll();
	}
}

class GetMaterialsByClass: NetApiHandler
{
    override JsonApiStruct GetRequest()
    {
        return new GetMaterialsByClassRequest();
    }

    override JsonApiStruct GetResponse(JsonApiStruct request)
    {
        GetMaterialsByClassRequest req = GetMaterialsByClassRequest.Cast(request);
        GetMaterialsByClassResponse response = new GetMaterialsByClassResponse();

        // Get addons
        array<string> addons = new array<string>;
        GameProject.GetLoadedAddons(addons);

        // Register Filter
        SearchResourcesFilter filter = new SearchResourcesFilter();
        filter.fileExtensions = {"emat"};

        for (int i = 0; i < addons.Count(); i++)
        {
            // Search Filter for every addon
            string addonName = GameProject.GetAddonID(addons[i]);
			string path = "$" + addonName + ":";
            filter.rootPath = path;
			// Get resources
            array<string> resources = {};
            ResourceDatabase.SearchResources(filter, resources.Insert);
            foreach (string resName : resources)
            {
                // Filter our resources
                Resource resource = Resource.Load(resName);
				if (!resource.IsValid()) continue;
                BaseContainer ematCont = resource.GetResource().ToBaseContainer();
				if (!ematCont) continue;
                string className = ematCont.GetClassName();

                if (className == req.matClass && !response.data.Contains(className))
                {
                    response.data.Insert(resName);
                }
            }
        }
		response.status = InfoStatus.OK;
        return response;
    }
}

/*
	IMPORT EMAT
*/

class MatUVTransformTemplate : JsonApiStruct
{
	float TilingU = 1;
	float TilingV = 1;
	float OffsetU = 0;
	float OffsetV = 0;
	float Rotation = 0;

	//----------------------------------------------------------------------------------------------
	void MatUVTransformTemplate()
	{
		RegAll();
	}
}

class MatParam : JsonApiStruct
{
	string name; 
	string value;
	
	//----------------------------------------------------------------------------------------------
	void MatParam()
	{
		RegAll();
	}
}

class DynMaterialTemplate : JsonApiStruct
{
	bool create = false;
	ResourceName resourceName;
	string resourcePath;
	string type; 
	ref array<ref MatParam> data = {}; 
	
	//----------------------------------------------------------------------------------------------
	void DynMaterialTemplate()
	{
		RegAll();
	}
}

class ExportMaterialResource : NetApiHandler
{
	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetRequest()
	{
		return new DynMaterialTemplate();
	}
	
	//----------------------------------------------------------------------------------------------
	static private void CopyParamsToEmatContainer(array<ref MatParam> data, notnull BaseContainer container)
	{
		foreach(MatParam param : data)
		{
			int index = container.GetVarIndex(param.name);
			
			if (index == -1)
			{
				Print(string.Format("%1 field was not found in EMAT!!!", param.name));
				continue;
			}
			
			if (param.value == "None" | param.value == string.Empty)
				{
					container.ClearVariable(param.name);
					continue;
				}
			
			if(param.name.Contains("UVTransform"))
			{
				MatUVTransformTemplate transform = new MatUVTransformTemplate;
				transform.ExpandFromRAW(param.value);
				
				Resource matUVTransformResource;
				BaseContainer matUVTransformContainer = container.GetObject(param.name);
				
				if (!matUVTransformContainer)
				{
					matUVTransformResource = BaseContainerTools.CreateContainer("MatUVTransform");
					matUVTransformContainer = matUVTransformResource.GetResource().ToBaseContainer();
				}
				
				BaseContainerTools.ReadFromInstance(transform, matUVTransformContainer);
				
				int numOfDefaultValues = 0;
				// Prevent serialization of values that equal to default value
				for (int i = 0; i < matUVTransformContainer.GetNumVars(); i++)
				{
					string varName = matUVTransformContainer.GetVarName(i);
					string defaultValue;
					string currentValue;
					matUVTransformContainer.GetDefaultAsString(varName, defaultValue);
					matUVTransformContainer.Get(varName, currentValue);
					
					if (defaultValue == currentValue)
					{
						matUVTransformContainer.ClearVariable(varName);
						numOfDefaultValues += 1;
					}
				}
				
				if (numOfDefaultValues == matUVTransformContainer.GetNumVars())
				{
					container.ClearVariable(param.name);
					continue;
				}
					
				container.SetObject(param.name, matUVTransformContainer); //in case of new instance of MatUVTransform
				continue;	
			}
			
			container.Set(param.name, param.value);
			
			// Prevent serialization of values that equal to default value
			// We do it after setting so that values have correct format e.g 1 instead of 1.000 etc...
			string defaultValue;
			string currentValue;
			container.GetDefaultAsString(param.name, defaultValue);
			container.Get(param.name, currentValue);
			
			if (defaultValue == currentValue)
			{
				container.ClearVariable(param.name);
			}
		}
	}
	
	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		DynMaterialTemplate req = DynMaterialTemplate.Cast(request);
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		
		string resourcePath;
		Resource resource;
		BaseContainer container;
		
		if (req.create)
		{
			resource = BaseContainerTools.CreateContainer(req.type);
			if (!resource)
				return ExportResult(ExportStatus.ERROR, string.Empty, string.Format("Error creating BaseContainer with type: %1", req.type));
			
			container = resource.GetResource().ToBaseContainer();
			resourcePath = req.resourcePath;
		}
		else
		{
			resourcePath = req.resourceName.GetPath();
			MetaFile meta = resourceManager.GetMetaFile(resourcePath);
		
			if (!meta)
				return ExportResult(ExportStatus.ERROR, string.Empty, string.Format("Resource not found: %1", req.resourceName));
			
			resource = Resource.Load(req.resourceName);
			container = resource.GetResource().ToBaseContainer();
		}
		
		CopyParamsToEmatContainer(req.data, container);
		
		BaseContainerTools.SaveContainer(container, "", resourcePath);
		
		if (req.create)
			resourceManager.RegisterResourceFile(resourcePath, false);
		
		return ExportResult(ExportStatus.OK, resourcePath);
	}
}

/*
	Animation Export Profile Info
*/

//----------------------------------------------------------------------------------------------
class AnimationProfileCollector
{
	ref array<string> m_Aprs = new array<string>();

	void OnFound(ResourceName resourceName, string exactPath)
	{		
		string absPath;
		if (!Workbench.GetAbsolutePath(exactPath, absPath, false) || absPath.IsEmpty())
			return;
		
		m_Aprs.Insert(absPath);
	}
}

//----------------------------------------------------------------------------------------------
static array<string> FindAnimationProfilesInProject()
{
	AnimationProfileCollector collector = new AnimationProfileCollector();

	SearchResourcesFilter filter = new SearchResourcesFilter();
	filter.fileExtensions = {"apr"};
	ResourceDatabase.SearchResources(filter, collector.OnFound);

	return collector.m_Aprs;
}

class GetAnimationExportProfilesResponse : JsonApiStruct
{
	ref array<string> animationExportProfiles = new array<string>;

	//----------------------------------------------------------------------------------------------
	void GetAnimationExportProfilesResponse(array<string> inAnimationExportProfiles)
	{
		animationExportProfiles = inAnimationExportProfiles;
		RegAll();
	}
}

class GetAnimationExportProfileInfosInProject : NetApiHandler
{
	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetRequest()
	{
		return null;
	}
	
	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		return new GetAnimationExportProfilesResponse(FindAnimationProfilesInProject());
	}
}

class AnimationExportProfileChannelInfo: JsonApiStruct
{
	string name;
	string genFn;
	
	//----------------------------------------------------------------------------------------------
	void AnimationExportProfileChannelInfo(string inName, string inGenFn)
	{
		name = inName;
		genFn = inGenFn;
		RegAll();
	}
}

class GetAnimationExportProfilesChannelsInfoRequest: JsonApiStruct
{
	string animationExportProfile;

	//----------------------------------------------------------------------------------------------
	void GetAnimationExportProfilesChannelsInfoRequest()
	{
		RegAll();
	}
}

class GetAnimationExportProfilesChannelsInfoResponse: JsonApiStruct
{
	string status;
	string message;
	ref array<ref AnimationExportProfileChannelInfo> channelInfos;

	//----------------------------------------------------------------------------------------------
	void GetAnimationExportProfilesChannelsInfoResponse(string inStatus, string inMessage, array<ref AnimationExportProfileChannelInfo> inChannelInfos)
	{
		status = inStatus;
		message = inMessage;
		channelInfos = inChannelInfos;
		RegAll();
	}
}


class GetAnimationExporProfileChannelsInfo : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new GetAnimationExportProfilesChannelsInfoRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		auto req = GetAnimationExportProfilesChannelsInfoRequest.Cast(request);
		
		string folderName = FilePath.StripFileName(req.animationExportProfile);
		string fileName = FilePath.StripPath(req.animationExportProfile);
		string profileName = FilePath.StripExtension(fileName);
		
		TxaExporter txaExporter = new TxaExporter();
		txaExporter.LoadProfiles(folderName);
		
		int profileIndex = txaExporter.GetProfileIndex(profileName);
		
		auto channelInfos = new array<ref AnimationExportProfileChannelInfo>;
		if (profileIndex == -1)
		{
			return new GetAnimationExportProfilesChannelsInfoResponse("error", "Requested profile does not exist", null);
		}
		
		else
		{
			for (int channelIndex = 0; channelIndex < txaExporter.GetProfileChannelCount(profileIndex); channelIndex++)
			{
				channelInfos.Insert(
					new AnimationExportProfileChannelInfo(
						txaExporter.GetProfileChannelName(profileIndex, channelIndex),
						txaExporter.GetProfileChannelGenFn(profileIndex, channelIndex)
					)
				);
			}
				
		}
		return new GetAnimationExportProfilesChannelsInfoResponse("", "", channelInfos);
	}
	
	
}


/*
	IMPORT FBX
*/

class MaterialOverride : JsonApiStruct
{
	string sourceMaterial;
	string assignedMaterial;
	
	//----------------------------------------------------------------------------------------------
	void MaterialOverride()
	{
		RegAll();
	}
}

class FBXImportRequest : JsonApiStruct
{
	string resourcePath;
	bool exportMorphs;
	bool exportSceneHierarchy;
	bool exportSkinning;
	string validationMode;
	ref array<ref MaterialOverride> materialOverrides = new array<ref MaterialOverride>;
	
	//----------------------------------------------------------------------------------------------
	void FBXImportRequest()
	{
		RegAll();
	}
}

class ExportFBXResource : NetApiHandler
{
	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetRequest()
	{
		return new FBXImportRequest();
	}
	
	//----------------------------------------------------------------------------------------------
	void SetWithClear(BaseContainer container, string varName, bool value)
	{
		string defaultValue, currentValue;
		
		container.Set(varName, value);
		container.Get(varName, currentValue); // stores bool as '0' and '1'
		container.GetDefaultAsString(varName, defaultValue);
		
		if (defaultValue == currentValue)
			container.ClearVariable(varName);
	}

	//----------------------------------------------------------------------------------------------
	void SetValidationModeTag(MetaFile meta, string validationMode)
		{
			const string permissiveTag = "EBTPermissiveExport";

			string tagsStr;
			meta.Get("Tags", tagsStr);

			array<string> tags = {};
			tagsStr.Split(" ", tags, true);

			bool tagsChanged = false;
			if (validationMode == "Normal")
			{
				tagsChanged = tags.RemoveItemOrdered(permissiveTag);
			}
			else if (validationMode == "Permissive")
			{
				if (!tags.Contains(permissiveTag))
				{
					tags.Insert(permissiveTag);
					tagsChanged = true;
				}
			}

			if (tagsChanged)
				meta.Set("Tags", string.Join(" ", tags, true));
		}

	
	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		FBXImportRequest req = FBXImportRequest.Cast(request);
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(req.resourcePath);
		
		if (!meta)
		{
			if (resourceManager.RegisterResourceFile(req.resourcePath, false))
				meta = resourceManager.GetMetaFile(req.resourcePath);			
		}

		if (!meta)
			return ExportResult(ExportStatus.ERROR, req.resourcePath, string.Format("Resource could not be registered: %1", req.resourcePath));

		BaseContainer cfg = meta.GetObjectArray("Configurations").Get(0);
		
		SetWithClear(cfg, "ExportMorphs", req.exportMorphs);
		SetWithClear(cfg, "ExportSceneHierarchy", req.exportSceneHierarchy);
		SetWithClear(cfg, "ExportSkinning", req.exportSkinning);
		
		cfg.ClearVariable("GeometryParams");
		
		
		if (req.materialOverrides.Count() > 0)
		{
			BaseContainerList materialAssigns = cfg.SetObjectArray("MaterialAssigns");
					
			foreach(MaterialOverride oRide : req.materialOverrides)
			{
				bool materialSourceFound;
				for (int i = 0; i < materialAssigns.Count(); i++)				
				{
					string sourceMaterial;
					materialAssigns[i].Get("SourceMaterial", sourceMaterial);
					
					if (sourceMaterial == oRide.sourceMaterial)
					{
						materialAssigns[i].Set("AssignedMaterial", oRide.assignedMaterial);
						materialSourceFound = true;
						break;
					}
				}
				
				if (materialSourceFound)
					continue;
		
				
				Resource materialAssign = BaseContainerTools.CreateContainer("MaterialAssignClass");
				
				BaseContainer cont = materialAssign.GetResource().ToBaseContainer();
				cont.Set("SourceMaterial", oRide.sourceMaterial);
				cont.Set("AssignedMaterial", oRide.assignedMaterial);
				
				materialAssigns.Insert(cont);
			}
			
			
		}
	
		SetValidationModeTag(meta, req.validationMode);
		
		meta.Save();

		resourceManager.RebuildResourceFile(req.resourcePath, "PC", false);
		return ExportResult(ExportStatus.OK, req.resourcePath);
	}
}

/*
	IMPORT TEXTURE
*/
class ExportTextureResourceRequest : JsonApiStruct
{
	string sourcePath;
	string destinationPath;
	
	//----------------------------------------------------------------------------------------------
	void ExportTextureResourceRequest()
	{
		RegAll();
	}
}

class ExportTextureResource : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new ExportTextureResourceRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		ExportTextureResourceRequest req = ExportTextureResourceRequest.Cast(request);
		
		if (!ResourceUtils.RegisterTextureResource(req.sourcePath, req.destinationPath))
			return ExportResult(ExportStatus.ERROR, req.destinationPath, "Failed to register texture");

		return ExportResult(ExportStatus.OK, req.destinationPath);
	}
}

/*
	TXA IMPORT
*/

enum TxaErrorCode
{
	OK = 0,
	WARNING = 1,
	ERROR = 2,
};


class ExportTXAResourceRequest : JsonApiStruct
{
	string	sourceFile;
	string	targetFile;
	string	profile;
	int		numberOfChannels;
	int		numberOfKeyframes;
	int		fps;
	ref array<float>	keyData = {};
	ref array<float>	diffData = {};
	
	//----------------------------------------------------------------------------------------------
	void ExportTXAResourceRequest()
	{
		RegAll();
	}
}


class ExportTXAResource : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new ExportTXAResourceRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		ExportTXAResourceRequest req = ExportTXAResourceRequest.Cast(request);
		
		TxaExporter txaExporter = new TxaExporter();
		
		
		MetaFile meta = GetMetaFileFromString(req.targetFile);
		ResourceName resourceName;
		string absolutePath;

		if (meta)
		{
			resourceName = meta.GetResourceName();
			Workbench.GetAbsolutePath(resourceName.GetPath(), absolutePath, true);
		}

		string profileDirectory = FilePath.StripFileName(req.profile);
		string profileName = FilePath.StripExtension(FilePath.StripPath(req.profile));

		TxaErrCode err = txaExporter.LoadProfiles(profileDirectory);
		if (err == TxaErrorCode.ERROR)
			return ExportResult(ExportStatus.ERROR, req.targetFile, txaExporter.ErrMsg());

		int profileIdx = txaExporter.GetProfileIndex(profileName);

		if (profileIdx == -1)
			return ExportResult(ExportStatus.ERROR, req.targetFile, "Missing export profile");

		err = txaExporter.TrackReset(profileIdx, req.numberOfKeyframes, req.fps, req.sourceFile, req.targetFile);

		if (err == TxaErrorCode.ERROR)
			return ExportResult(ExportStatus.ERROR, req.targetFile, txaExporter.ErrMsg());

		err = txaExporter.TrackSetChannels(req.numberOfChannels, req.numberOfKeyframes, req.keyData, req.diffData);
		if (err == TxaErrorCode.ERROR)
			return ExportResult(ExportStatus.ERROR, req.targetFile, txaExporter.ErrMsg());
		
		err = txaExporter.TrackExport();
		if (err == TxaErrorCode.ERROR)
			return ExportResult(ExportStatus.ERROR, req.targetFile, txaExporter.ErrMsg());
		
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		
		if (!meta)
		{
			resourceManager.RegisterResourceFile(req.targetFile, false);
			meta = GetMetaFileFromString(req.targetFile);
		}
			
		if (!meta)
			return ExportResult(ExportStatus.ERROR, req.targetFile, "Could not register TXA resource");
		
		resourceManager.RebuildResourceFile(req.targetFile, "PC", false);
		return ExportResult(ExportStatus.OK,req.targetFile);
	}
}

/*
	GENERAL IMPORT
*/

class RegisterResourceRequest : JsonApiStruct
{
	ref array<string> path = new array<string>;
	
	void RegisterResourceRequest()
	{
		RegV("path");
	}
}

class RegisterResourceResponse : JsonApiStruct
{
	bool Output;

	void RegisterResourceResponse()
	{
		RegV("Output");
	}
}

class RegisterResource : NetApiHandler
{
	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetRequest()
	{
		return new RegisterResourceRequest();
	}
	
	//----------------------------------------------------------------------------------------------
	bool Register(string absPath)
	{
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		MetaFile meta = rm.GetMetaFile(absPath);
		
		// check if meta doesnt exist already
		if (!meta)
		{
			// create if not
			if (rm.RegisterResourceFile(absPath, false))
				meta = rm.GetMetaFile(absPath);
				
			// if creation was not succesful
			if (!meta)	
				return false;
			
			// build xob file if registration was successful
			rm.RebuildResourceFile(absPath, "PC", true);
		}
	
		// Overwrite meta file configs
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer cfg = configurations.Get(0);
		
		// For texture
		if (IsImage(configurations[0].GetClassName()))
		{
			TextureTypes textureTypes = new TextureTypes();
			textureTypes.DoChecks(TextureIssueOp.Fix, meta.GetResourceID(), meta);
			meta.Save();
			return true;
		}
		
		return true;
	}

	//----------------------------------------------------------------------------------------------
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		RegisterResourceRequest req = RegisterResourceRequest.Cast(request);
		RegisterResourceResponse response = new RegisterResourceResponse();

		// for each export path
		for (int i = 0; i < req.path.Count(); i++)
		{
			response.Output = Register(req.path[i]);
		}
		return response;
	}
}


/*
	Selected Entities
*/


class SelectedEntityInfo : JsonApiStruct
{
	ResourceName resourceName;
	ref array<float> transform = {};

	void SelectedEntityInfo()
	{
		RegAll();
	}
}

class GetSelectedEntitiesResponse : JsonApiStruct
{
	ref array<ref SelectedEntityInfo> entities = {};

	void GetSelectedEntitiesResponse()
	{
		RegAll();
	}
}

class GetSelectedEntities : NetApiHandler
{
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetSelectedEntitiesResponse response = new GetSelectedEntitiesResponse();
		WorldEditor we = Workbench.GetModule(WorldEditor);
		auto api = we.GetApi();

		int count = api.GetSelectedEntitiesCount();

		for (int i = 0; i < count; i++)
		{
			IEntitySource entity_source = api.GetSelectedEntity(i);
			
			if (!entity_source)
				continue;

			IEntity entity = api.SourceToEntity(entity_source);

			if (!entity)
				continue;
			
			auto ancestor = entity_source.GetAncestor();
			
			if (!ancestor || !ancestor.GetResourceName())  
			{
				continue;
			}
	
			SelectedEntityInfo info = new SelectedEntityInfo();

			vector matrix[4];
			entity.GetWorldTransform(matrix);

			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 3; k++) 
				{
					info.transform.Insert(matrix[j][k]);
				}
			}
			
			info.resourceName = ancestor.GetResourceName();
			
			response.entities.Insert(info);
		}

		return response;
	}
}


/*
	Terrain
*/

class GetTerrainInfoRequest : JsonApiStruct
{
	int terrainIndex;

	void GetTerrainInfoRequest()
	{
		RegAll();
	}
}

class GetTerrainInfoResponse : JsonApiStruct
{
	int resolutionX;
	int resolutionY;
	int tilesX;
	int tilesY;
	float unitScale;

	void GetTerrainInfoResponse()
	{
		RegAll();
	}
}

class GetTerrainInfo : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new GetTerrainInfoRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetTerrainInfoRequest req = GetTerrainInfoRequest.Cast(request);
		GetTerrainInfoResponse response = new GetTerrainInfoResponse();
		WorldEditor we = Workbench.GetModule(WorldEditor);

		if (!we) 
		{
			return response;
		}

		auto api = we.GetApi();

		if (!api) 
		{
			return response;
		}

		response.resolutionX = api.GetTerrainResolutionX(req.terrainIndex);
		response.resolutionY = api.GetTerrainResolutionY(req.terrainIndex);
		response.tilesX = api.GetTerrainTilesX(req.terrainIndex);
		response.tilesY = api.GetTerrainTilesY(req.terrainIndex);
		response.unitScale = api.GetTerrainUnitScale(req.terrainIndex);

		return response;
	}
}

class GetTileHeightmapRequest : JsonApiStruct
{
	int terrainIndex;
	int tileX;
	int tileY;

	void GetTileHeightmapRequest()
	{
		RegAll();
	}
}

class GetTileHeightmapResponse : JsonApiStruct
{
	ref array<float> heightMap = {};

	void GetTileHeightmapResponse()
	{
		RegAll();
	}
}

class GetTileHeightmap : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new GetTileHeightmapRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetTileHeightmapRequest req = GetTileHeightmapRequest.Cast(request);
		GetTileHeightmapResponse response = new GetTileHeightmapResponse();
		WorldEditor we = Workbench.GetModule(WorldEditor);

		if (!we)
		{
			return response;
		}

		auto api = we.GetApi();

		if (!api) 
		{
			return response;
		}

		api.BeginTerrainAction();
		api.GetTerrainSurfaceTile(req.terrainIndex, req.tileX, req.tileY, response.heightMap);
		api.EndTerrainAction();

		return response;
	}
}


/*
	Satellite Map
*/

class GetSurfaceMaterialsRequest : JsonApiStruct
{
	int terrainIndex;

	void GetSurfaceMaterialsRequest()
	{
		RegAll();
	}
}

class GetSurfaceMaterialsResponse : JsonApiStruct
{
	ref array<string> materials = new array<string>();

	void GetSurfaceMaterialsResponse()
	{
		RegAll();
	}
}

class GetSurfaceMaterials : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new GetSurfaceMaterialsRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetSurfaceMaterialsRequest req = GetSurfaceMaterialsRequest.Cast(request);
		GetSurfaceMaterialsResponse response = new GetSurfaceMaterialsResponse();

		WorldEditor we = Workbench.GetModule(WorldEditor);
		if (!we) 
		{
			return null;	
		}

		WorldEditorAPI api = we.GetApi();
		if (!api) 
		{
			return null;
		}

		array<ResourceName> paths = new array<ResourceName>();
		api.GetSurfaceMaterials(req.terrainIndex, paths);

		foreach (ResourceName rn : paths)
		{
			response.materials.Insert(rn);
		}

		return response;
	}
}


class ExportSurfaceMasksRequest : JsonApiStruct
{
	int terrainIndex;
	string exportDirectory;

	void ExportSurfaceMasksRequest()
	{
		RegAll();
	}
}

class ExportSurfaceMasksResponse : JsonApiStruct
{
	bool success;

	void ExportSurfaceMasksResponse()
	{
		RegAll();
	}
}

class ExportSurfaceMasks : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new ExportSurfaceMasksRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		ExportSurfaceMasksRequest req = ExportSurfaceMasksRequest.Cast(request);
		ExportSurfaceMasksResponse response = new ExportSurfaceMasksResponse();

		WorldEditor we = Workbench.GetModule(WorldEditor);
		
		if (!we) 
		{
			return null;	
		}

		WorldEditorAPI api = we.GetApi();

		if (!api) 
		{
			return null;
		}

		response.success = api.ExportSurfaceMasks(req.terrainIndex, req.exportDirectory);
		return response;
	}
}

class ResourceUtils
{
	static MetaFile GetMetaFileForPath(string absPath)
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		return resourceManager.GetMetaFile(absPath);
	}

	static string GetAbsolutePathForResourceName(ResourceName resourceName)
	{
		string absPath;
		if (Workbench.GetAbsolutePath(resourceName.GetPath(), absPath))
			return absPath;
		
		return string.Empty;
	}

	static MetaFile GetMetaFileForResourceName(ResourceName resourceName)
	{
		string absPath = ResourceUtils.GetAbsolutePathForResourceName(resourceName);

		if (absPath == string.Empty)
			return null;

		return GetMetaFileForPath(absPath);
	}

	static ResourceName GetResourceNameForAbsolutePath(string absPath)
	{
		MetaFile meta = ResourceUtils.GetMetaFileForPath(absPath);

		if (!meta)
			return string.Empty;
		
		return meta.GetResourceID();
	}

	static map<string, ResourceName> GetMaterialAssignsFromXOBMeta(notnull MetaFile meta)
	{
		map<string, ResourceName> materialMap = new map<string, ResourceName>();

		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		if (!configurations || configurations.Count() == 0)
			return materialMap;

		BaseContainer cfg = configurations.Get(0);
		if (!cfg)
			return materialMap;

		BaseContainerList materialAssigns = cfg.GetObjectArray("MaterialAssigns");
		if (!materialAssigns)
			return materialMap;

		for (int i = 0, count = materialAssigns.Count(); i < count; i++)
		{
			BaseContainer materialAssign = materialAssigns.Get(i);
			if (!materialAssign)
				continue;

			string sourceMaterial;
			if (!materialAssign.Get("SourceMaterial", sourceMaterial) || sourceMaterial == string.Empty)
				continue;

			ResourceName assignedMaterial;
			materialAssign.Get("AssignedMaterial", assignedMaterial);
			materialMap.Set(sourceMaterial, assignedMaterial);
		}

		return materialMap;
	}

	static bool GetMlodMaterialResourceFromXOBMeta(notnull MetaFile meta, out ResourceName mlodMaterial)
	{
		mlodMaterial = string.Empty;

		map<string, ResourceName> materialMap = GetMaterialAssignsFromXOBMeta(meta);

		int foundCount = 0;

		foreach (string sourceMaterial, ResourceName assignedMaterial : materialMap)
		{
			if (!assignedMaterial || !assignedMaterial.GetPath())
				continue;

			string materialPath = FilePath.StripExtension(assignedMaterial.GetPath());
			materialPath.ToLower();

			// Interior MLODs are never baked into, so they have to be skipped before the "mlod" suffix test.
			if (materialPath.Contains("int_mlod") || materialPath.Contains("mlod_int"))
				continue;

			if (!materialPath.EndsWith("mlod"))
				continue;

			// Found an MLOD assigned material -> validate its source material name.
			string sourceName = sourceMaterial;
			sourceName.ToLower();

			if (!sourceName.Contains("mlod"))
				Print(string.Format("MLOD material '%1' has source material '%2' without 'mlod' in its name.", assignedMaterial.GetPath(), sourceMaterial), LogLevel.WARNING);

			foundCount++;

			if (foundCount == 1)
				mlodMaterial = assignedMaterial; // Keep the first.
			else
				Print(string.Format("Multiple MLOD materials found in XOB; ignoring '%1' and using the first.", assignedMaterial.GetPath()), LogLevel.WARNING);
		}

		return foundCount > 0;
	}

	static string GetMaterialClassName(ResourceName materialResource)
	{
		if (!materialResource || !materialResource.GetPath())
			return string.Empty;

		Resource resource = Resource.Load(materialResource);
		if (!resource || !resource.IsValid())
		{
			Print(string.Format("Could not load material resource '%1'.", materialResource.GetPath()), LogLevel.WARNING);
			return string.Empty;
		}

		BaseContainer material = resource.GetResource().ToBaseContainer();
		if (!material)
		{
			Print(string.Format("Material resource '%1' has no container.", materialResource.GetPath()), LogLevel.WARNING);
			return string.Empty;
		}

		return material.GetClassName();
	}

	static const int RESOURCE_WAIT_TIMEOUT_MS = 30000;

	static bool WaitForResourceFile(string absPath, int timeoutMs = RESOURCE_WAIT_TIMEOUT_MS)
	{
		if (absPath == string.Empty)
			return false;

		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		int elapsedMs = 0;
		while (!resourceManager.WaitForFile(absPath, 1000))
		{
			Sleep(100);
			elapsedMs += 1100;

			if (elapsedMs >= timeoutMs)
			{
				Print(string.Format("Gave up after %1 ms waiting for RDB file: %2", elapsedMs, absPath), LogLevel.ERROR);
				return false;
			}
		}

		Print(string.Format("Waited %1 ms for RDB file: %2", elapsedMs, absPath), LogLevel.NORMAL);
		return true;
	}

	
	static ResourceName GetTextureResourceFromMaterialSlot(ResourceName materialResource, string slotName)
	{
		if (materialResource == string.Empty || slotName == string.Empty)
			return string.Empty;

		Resource resource = BaseContainerTools.LoadContainer(materialResource);
		if (!resource || !resource.IsValid())
			return string.Empty;

		BaseContainer materialContainer = resource.GetResource().ToBaseContainer();
		if (!materialContainer || materialContainer.GetVarIndex(slotName) == -1)
			return string.Empty;

		string value;
		materialContainer.Get(slotName, value);

		if (value == string.Empty || value == " 0")
			return string.Empty;

		array<string> tokens = {};
		value.Split(" ", tokens, true);

		if (tokens.IsEmpty())
			return string.Empty;

		return tokens[0];
	}

	/*!
	Copies an image to its destination inside the project and registers it as a texture resource.
	\param sourcePath Absolute path of the source image
	\param destinationPath Absolute path to copy the image to, may be the source path itself
	*/
	static bool RegisterTextureResource(string sourcePath, string destinationPath)
	{
		if (sourcePath == string.Empty)
			return false;

		if (destinationPath == string.Empty)
			return false;
		
		if (sourcePath != destinationPath)
		{
			FileIO.CopyFile(sourcePath, destinationPath);
		}

		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);

		MetaFile meta = resourceManager.GetMetaFile(destinationPath);
		if (!meta)
		{
			if (resourceManager.RegisterResourceFile(destinationPath, false))
				meta = resourceManager.GetMetaFile(destinationPath);
		}

		if (!meta)
			return false;

		TextureTypes textureTypes = new TextureTypes();
		textureTypes.DoChecks(TextureIssueOp.Fix, meta.GetResourceID(), meta);
		meta.Save();

		resourceManager.RebuildResourceFile(destinationPath, "PC", false);
		return true;
	}
}
