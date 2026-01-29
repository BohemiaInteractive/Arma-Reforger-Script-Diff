/*
	EXPORT
*/
enum BlendMode 
{
	None = 0,
	ColorModulate = 4,
	AlphaBlend = 7,
	Additive =  5
}

enum Cull
{
	ccw = 0,
	none = 1
}

enum GlobalMCRMode
{
	Overlay = 0,
	Multiply = 1
}

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

class ContainerJSONSerializer
{
	//----------------------------------------------------------------------------------------------
	static string ResolveEnumValue(typename type, int enumValue)
	{
		switch (type)
		{
			case BlendMode:
			{
				return typename.EnumToString(BlendMode, enumValue);
			}
			case Cull:
			{
				return typename.EnumToString(Cull, enumValue);
			}
			case GlobalMCRMode:
			{
				return typename.EnumToString(GlobalMCRMode, enumValue);
			}
		}
		return "";
		
	}
	
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
							ResourceInfo resourceInfo = ResourceInfo.CreateFromResourceName(value, expand);
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
							ResourceInfo resourceInfo = ResourceInfo.CreateFromResourceName(value, expand);
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
					
					typename type = varName.ToType();
					
					if (!type)
					{
						jsonStruct.StoreInteger(varName, value);
						break;
					}
					
					string enumFieldName = ResolveEnumValue(type, value);
					jsonStruct.StoreString(varName, enumFieldName);
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
						jsonStruct.StoreObject(varName, new GenericJSONContainer(object, false, expand));
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
						jsonStruct.ItemObject(new GenericJSONContainer(objects.Get(objIdx), true, expand));
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
	bool m_bAsArrayItem = false;
	bool m_bExpandResource = false;

	//----------------------------------------------------------------------------------------------
	void GenericJSONContainer(BaseContainer container, bool asArrayItem = false, bool expand = false)
	{
		m_Container = container;
		m_bAsArrayItem = asArrayItem;
		m_bExpandResource = expand;
	}

	//----------------------------------------------------------------------------------------------
	override void OnPack()
	{
		string objectName = string.Format("%1", m_Container.GetClassName());
		if (m_bAsArrayItem)
		{
			StoreString("ClassName", objectName);
			StoreString("Name", m_Container.GetName());
			ContainerJSONSerializer.WriteVars(this, m_Container, m_bExpandResource);
		}
		else
		{
			StartObject(objectName);
				ContainerJSONSerializer.WriteVars(this, m_Container, m_bExpandResource);
			EndObject();
		}
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
	private string status;
	private string message;
	private string resourceName;
	private ref JsonApiStruct data;

	//----------------------------------------------------------------------------------------------
	void ResourceInfo(InfoStatus _status, ResourceName _resourceName, string _message, JsonApiStruct _data)
	{
		status = typename.EnumToString(InfoStatus, _status);
		resourceName = _resourceName;
		message = _message;
		data = _data;
	
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
	/*!
	ResourceInfo Factory. Expects relative path.
	*/
	static ResourceInfo CreateFromResourceName(ResourceName _resourceName, bool expand = false)
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		string resourcePath = _resourceName.GetPath();
		MetaFile meta = resourceManager.GetMetaFile(resourcePath);
	
		if (!meta)
		{
			return ResourceInfo(
			InfoStatus.ERROR, 
			string.Empty,
			string.Format("Resource not found: %1", _resourceName), 
			null);
		}
		
		BaseContainer configurationContainer = meta.GetObjectArray("Configurations")[0];
		
		if (!configurationContainer)
		{
			return ResourceInfo(
				InfoStatus.ERROR,
				_resourceName,
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
					CreateSourceFileNotFoundmessage(_resourceName),
					null);
	 		}
			
	 		return ResourceInfo(InfoStatus.OK, _resourceName, string.Empty, new XOBResourceInfo(configurationContainer, fbxSourcePath, expand));
		}
		
		if (className == "EMATResourceClass")
		{
			Resource resource = Resource.Load(_resourceName);
			
	 		if (!resource.IsValid())
	 		{
	 			return ResourceInfo(
					InfoStatus.ERROR, 
					_resourceName,
					string.Format("Resource could not be loaded: %1", _resourceName),
					null);
			}
			
	 		return ResourceInfo(InfoStatus.OK, _resourceName, string.Empty, new MaterialResourceInfo(_resourceName, expand));
		}
		
		
		if(className == "EntityTemplateResourceClass" || className == "GameMaterialResourceClass")
		{
			Resource resource = Resource.Load(_resourceName);
			
	 		if (!resource.IsValid())
	 		{
	 			return ResourceInfo(
					InfoStatus.ERROR, 
					_resourceName,
					string.Format("Resource could not be loaded: %1", _resourceName),
					null);
			}
			
			if (className == "GameMaterialResourceClass") //Hack to prevent recursion
				expand = false;
			
	 		return ResourceInfo(InfoStatus.OK, _resourceName, "", new GenericResourceInfo(_resourceName, expand));
		}
		
		
		if (IsImage(className))
		{
			string textureSourcePath;
	 		if (!Workbench.GetAbsolutePath(meta.GetSourceFilePath(), textureSourcePath, true))
	 		{
	 			return ResourceInfo(
					InfoStatus.ERROR, 
					_resourceName,
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
			
	 		return ResourceInfo(InfoStatus.OK, _resourceName, string.Empty, textureProperty);
		}
		
		return ResourceInfo(
			InfoStatus.ERROR, 
			_resourceName,
			string.Format("Unsuported resource type: %1 for resource: %2", className, _resourceName),
			null); 
	}
	
	//----------------------------------------------------------------------------------------------
	static ResourceInfo CreateFromResourceAbsolutePath(string absPath, bool expand = false)
	{
		if (!FileIO.FileExists(absPath))
		{
			return ResourceInfo(
			InfoStatus.ERROR, 
			string.Empty,
			string.Format("Resource with absolute path: %1 not found. ", absPath), 
			null);
		}
		
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(absPath);
		
		if (!meta)
		{
			return ResourceInfo(
			InfoStatus.ERROR, 
			string.Empty,
			string.Format("Resource with absolute path: %1 is not registered with current runing project. ", absPath), 
			null);
		}
		
		return CreateFromResourceName(meta.GetResourceID(), expand);
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
		
		if (FilePath.IsAbsolutePath(req.path))
		{
			return ResourceInfo.CreateFromResourceAbsolutePath(req.path, req.expandResource);
		}
		
		return ResourceInfo.CreateFromResourceName(req.path, req.expandResource);
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
			string.Format("Resource not found: %1", req.resourceName), 
			null);
		}
		
		Resource resource = Resource.Load(req.resourceName);
		BaseContainer baseContainer = resource.GetResource().ToBaseContainer();
		
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
			string.Format("Prefab doesn't have child with index: %1", req.childIdx), 
			null);
		}
		
		GenericJSONContainer data = new GenericJSONContainer(childContainer, false, req.expandResource);
		
		return ResourceInfo(InfoStatus.OK, "", string.Empty, data);
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
			{
				return ResourceInfo(
				InfoStatus.ERROR, 
				string.Empty,
				string.Format("Error creating BaseContainer with type: %1", req.type), 
				null);
			}
			
			container = resource.GetResource().ToBaseContainer();
			resourcePath = req.resourcePath;
		}
		else
		{
			resourcePath = req.resourceName.GetPath();
			MetaFile meta = resourceManager.GetMetaFile(resourcePath);
		
			if (!meta)
			{
				return ResourceInfo(
				InfoStatus.ERROR, 
				string.Empty,
				string.Format("Resource not found: %1", req.resourceName), 
				null);
			}
			
			resource = Resource.Load(req.resourceName);
			container = resource.GetResource().ToBaseContainer();
		}
		
		CopyParamsToEmatContainer(req.data, container);
		
		BaseContainerTools.SaveContainer(container, "", resourcePath);
		
		if (req.create)
			resourceManager.RegisterResourceFile(resourcePath, false);
		
		return ResourceInfo.CreateFromResourceAbsolutePath(resourcePath);
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
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		FBXImportRequest req = FBXImportRequest.Cast(request);
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(req.resourcePath);
		
		if (!meta)
		{
			if (resourceManager.RegisterResourceFile(req.resourcePath, false))
				meta = resourceManager.GetMetaFile(req.resourcePath);
					
			if (!meta)	
			{
				return ResourceInfo(
				InfoStatus.ERROR, 
				string.Empty,
				string.Format("Resource could not be registered: %1", req.resourcePath), 
				null);
			}
		}
		
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
		
		meta.Save();

		resourceManager.RebuildResourceFile(req.resourcePath, "PC", false);
		return ResourceInfo.CreateFromResourceAbsolutePath(req.resourcePath);	
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
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
	    
		string resourcePath = "";
	    
		if (req.sourcePath == "")
		{
			return ResourceInfo(
			InfoStatus.ERROR, 
			string.Empty,
			string.Format("Resource could not be registered (Missing source path!): %1", resourcePath), 
			null);
		}
		
		else if (req.destinationPath == "")
		{
			return ResourceInfo(
			InfoStatus.ERROR, 
			string.Empty,
			string.Format("Resource could not be registered (Missing destination path!): %1", resourcePath), 
			null);
		}
		
		else if (req.sourcePath == req.destinationPath)
	    {
	        resourcePath = req.sourcePath;
	    }
	    else
	    {
	        FileIO.CopyFile(req.sourcePath, req.destinationPath);
	        resourcePath = req.destinationPath;
	    }
				
		// TODO: check if path is in project
		// TODO: do not copy if destination is outside of project
		
		MetaFile meta = resourceManager.GetMetaFile(resourcePath);

		if (!meta)
		{
			if (resourceManager.RegisterResourceFile(resourcePath, false))
				meta = resourceManager.GetMetaFile(resourcePath);
					
			if (!meta)	
			{
				return ResourceInfo(
				InfoStatus.ERROR, 
				string.Empty,
				string.Format("Resource could not be registered: %1", resourcePath), 
				null);
			}
		}
	
		TextureTypes textureTypes = new TextureTypes();
		textureTypes.DoChecks(TextureIssueOp.Fix, meta.GetResourceID(), meta);
		meta.Save();

		resourceManager.RebuildResourceFile(resourcePath, "PC", false);
		return ResourceInfo.CreateFromResourceAbsolutePath(resourcePath);	
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