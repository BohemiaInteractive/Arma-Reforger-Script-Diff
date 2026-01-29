#ifdef WORKBENCH

static const int TEXTURE_SIZES[3] = {512, 1024, 2048};
static const int TEXTURE_IMPORT_SIZES[4] = {-1, 256, 512, 1024};
static const int SUPER_SAMPLING[3] = {1, 2, 4};

class ParamEnumTextureSize : array<ref ParamEnum>
{
	static ParamEnumArray FromEnum()
	{
		ParamEnumArray params = new ParamEnumArray();

		for (int i, max = StaticArray.Length(TEXTURE_SIZES); i < max; i++)
		{
			params.Insert(new ParamEnum(TEXTURE_SIZES[i].ToString(), i.ToString()));
		}

		return params;
	}
}

class ParamEnumTextureImportSize : array<ref ParamEnum>
{
	static ParamEnumArray FromEnum()
	{
		ParamEnumArray params = new ParamEnumArray();

		params.Insert(new ParamEnum("No limit", "0"));

		for (int i = 1, max = StaticArray.Length(TEXTURE_IMPORT_SIZES); i < max; i++)
		{
			params.Insert(new ParamEnum(TEXTURE_IMPORT_SIZES[i].ToString(), i.ToString()));
		}

		return params;
	}
}

class ParamEnumSuperSampling : array<ref ParamEnum>
{
	static ParamEnumArray FromEnum()
	{
		ParamEnumArray params = new ParamEnumArray();

		params.Insert(new ParamEnum("None", "0"));
		params.Insert(new ParamEnum("2x", "1"));
		params.Insert(new ParamEnum("4x", "2"));

		return params;
	}
}

enum FixRayMissType
{
	FAST,
	SMOOTH_CAGE_BAKE
}

enum FixRayMissExtents
{
	BROAD,
	MINIMUM
}

[WorkbenchPluginAttribute(name: "EBT: Bake MLOD", wbModules: { "ResourceManager" }, resourceTypes: { "fbx", "xob"	 }, awesomeFontCode: 0xf7ec)]
class BakeMLODPlugin : WorkbenchPlugin
{
	[Attribute("16", UIWidgets.EditBox, "", category : "Settings")]
	protected int m_iNumberOfSamples;

	[Attribute("True", UIWidgets.EditBox, "", category : "Development")]
	protected bool m_bRunInBackground = true;

	//------------------------------------------------------------------------------------------------
	protected static ref StartBakeDialogXOB s_StartBakeDialogXOB = new StartBakeDialogXOB("Bake XOB MLOD");

	static ref StartBakeDialogET s_StartBakeDialogET = new StartBakeDialogET("Bake ET MLOD");

	protected static BakeMLODPlugin s_Instance;

	void BakeMLODPlugin()
	{
		if (!s_Instance)
			s_Instance = this;
	}

	static BakeMLODPlugin GetInstance()
	{
		return s_Instance;
	}

	static bool GetOperatorDescription(StartBakeDialog startBakeDialog, ResourceName resourceName, int noOfSamples, out BlenderOperatorDescription operatorDescription)
	{
		string pathToResourceParam;
		Workbench.GetAbsolutePath(resourceName.GetPath(), pathToResourceParam);

		if (resourceName.EndsWith(".xob"))
			pathToResourceParam.Replace(".xob", ".fbx");

		if (!FileIO.FileExists(pathToResourceParam))
		{
			Print("Resource couldn't be imported to blender because .fbx file is missing", LogLevel.WARNING);
			return false;
		}

		//Common params
		int textureSize= TEXTURE_SIZES[startBakeDialog.m_iTextureSize];
		int textureImportSize= TEXTURE_IMPORT_SIZES[startBakeDialog.m_iTextureImportSize];
		int superSamplingSize = SUPER_SAMPLING[startBakeDialog.m_iSuperSampling];

		operatorDescription.AddParam("file_path", pathToResourceParam);
		operatorDescription.AddParam("clear_color", startBakeDialog.m_ClearColor);
		operatorDescription.AddParam("texture_size", textureSize);
		operatorDescription.AddParam("texture_import_size", textureImportSize);
		operatorDescription.AddParam("cage_extrusion", startBakeDialog.m_fCageExtrusion);
		operatorDescription.AddParam("ray_distance", startBakeDialog.m_fRayDistance);
		operatorDescription.AddParam("skip_interior", startBakeDialog.m_bSkipInterior);
		operatorDescription.AddParam("no_of_samples", noOfSamples);
		operatorDescription.AddParam("fix_ray_miss_type", startBakeDialog.m_eFixRayType);
		operatorDescription.AddParam("fix_ray_miss_extents", startBakeDialog.m_eFixRayExtents);
		operatorDescription.AddParam("super_sampling", superSamplingSize);
		
		
		if (startBakeDialog.Type() == StartBakeDialogXOB)
		{
			StartBakeDialogXOB dialog = StartBakeDialogXOB.Cast(startBakeDialog);
			
			string absTexturePath = "";
			// When you wanna get absolute path but no param, then it will give you the root of... The last addon?
			if(dialog.m_sTexturePath != "")
			{
				Workbench.GetAbsolutePath(dialog.m_sTexturePath,absTexturePath);
			}
			operatorDescription.blIDName = "ebt.bake_workbench_background_xob_mlod";
			operatorDescription.AddParam("re_uv", dialog.m_bReUV);
			operatorDescription.AddParam("skip_transparent", dialog.m_bSkipTransparent);
			operatorDescription.AddParam("create_new_texture", dialog.m_bCreateNewTexture);
			operatorDescription.AddParam("texture_name",dialog.m_sTextureName);
			operatorDescription.AddParam("texture_path",absTexturePath);
		}
		else
		{
			string baseFbx;
			if (!PrefabImporterBake.GetBaseFBX(resourceName, baseFbx))
			{
				Print("Could not get Base FBX", LogLevel.WARNING);
				return false;
			}
			
			operatorDescription.blIDName  = "ebt.bake_workbench_background_et_mlod";
			operatorDescription.AddParam("base_fbx",  baseFbx);
			operatorDescription.AddParam("new_texture", StartBakeDialogET.Cast(startBakeDialog).m_sNewTextureName);
		}
		
		return true;
	}
	
	
	void StartBake(ResourceName resource)
	{
		if (!EBTConfigPlugin.HasBlenderRegistered())
			return;
		
		StartBakeDialog bakeDialog = s_StartBakeDialogET;

		if (resource.EndsWith(".xob") || resource.EndsWith(".fbx"))
			bakeDialog = s_StartBakeDialogXOB;

		if (!Workbench.ScriptDialog(bakeDialog.m_sTitle, "Make sure you have supported version of Blender and newest EBT installed.", bakeDialog))
			return;


		string pathToExecutable;
		if (!EBTConfigPlugin.GetDefaultBlenderPath(pathToExecutable))
			return;

		BlenderOperatorDescription operatorDescription = new BlenderOperatorDescription("bake");

		if (GetOperatorDescription(bakeDialog, resource, m_iNumberOfSamples, operatorDescription))
		{
				StartBlenderWithOperator(operatorDescription, m_bRunInBackground);
				return;
		}
		
		Print("Could not create a operatorDescription", LogLevel.WARNING);
		
	}
	
	void VoidPrint(void param)
	{
		Print(param);
	}

	//------------------------------------------------------------------------------------------------
	override void OnResourceContextMenu(notnull array<ResourceName> resources)
	{
		if (resources.IsEmpty())
		{
			Print("No selected resources", LogLevel.WARNING);
			return;
		}

		StartBake(resources[0]);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	protected void ButtonOK() {}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("MLOD Baker Settings", "", this);
	}
}

class StartBakeDialog
{
	[Attribute("1024", UIWidgets.ComboBox, "", "", enums: ParamEnumTextureSize.FromEnum(), category : "Settings")]
	int m_iTextureSize;

	//Hidden for now
	//[Attribute("0", UIWidgets.ComboBox, "", "", enums: ParamEnumSuperSampling.FromEnum(), category : "Settings")]
	int m_iSuperSampling;

	[Attribute("1024", UIWidgets.ComboBox, "", "", enums: ParamEnumTextureImportSize.FromEnum(), category : "Settings")]
	int m_iTextureImportSize;

	[Attribute("0.5 0.5 0.5 1", UIWidgets.ColorPicker, "", category: "Settings")]
	ref Color m_ClearColor = new Color(0.5, 0.5, 0.5, 1);

	[Attribute("2.5", UIWidgets.EditBox, "", category: "Settings")]
	float m_fCageExtrusion = 2.5;

	[Attribute("0.3", UIWidgets.EditBox, "", category: "Settings")]
	float m_fRayDistance = 0.3;

	[Attribute("0", UIWidgets.ComboBox, "", "", enums: ParamEnumArray.FromEnum(FixRayMissType), category : "Settings")]
	FixRayMissType m_eFixRayType;

	[Attribute("0", UIWidgets.ComboBox, "", "", enums: ParamEnumArray.FromEnum(FixRayMissExtents), category : "Settings")]
	FixRayMissExtents m_eFixRayExtents;

	[Attribute("False", UIWidgets.EditBox, "", category : "Settings")]
	bool m_bSkipInterior;


	string m_sTitle = "";

	//------------------------------------------------------------------------------------------------
	void StartBakeDialog(string title)
	{
		m_sTitle = title;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	bool OkButton()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	bool ButtonCancel()
	{
		return false;
	}
}

class StartBakeDialogXOB : StartBakeDialog
{
	[Attribute("False", UIWidgets.EditBox, "", category : "Settings")]
	bool m_bSkipTransparent;

	[Attribute("False", UIWidgets.EditBox, "", category : "Settings")]
	bool m_bReUV;
	
	[Attribute(uiwidget: UIWidgets.FileNamePicker, desc: "Root directory to search", params:"unregFolders", category : "New Texture")]
	string m_sTexturePath;
	
	[Attribute("", UIWidgets.Auto, desc:"Name of the new texture",category: "New Texture")]
	string m_sTextureName;
	
	[Attribute("", UIWidgets.EditBox, "", category: "Settings")]
	bool m_bCreateNewTexture;
	

}

class StartBakeDialogET : StartBakeDialog
{
	[Attribute("", UIWidgets.EditBox, "", category: "Settings")]
	string m_sNewTextureName;
}

class BakeInfoRequest : JsonApiStruct
{
	string msg;

	void BakeInfoRequest()
	{
		RegV("msg");
	}
}

static ResourceName GetOpenedPrefab(WorldEditor worldEditor, WorldEditorAPI api)
{	
	IEntitySource entitySource = null;
	
	if (worldEditor.IsPrefabEditMode() && api.GetEditorEntityCount() == 2)
		entitySource = api.GetEditorEntity(1);		
	
	if (entitySource)
	{
		BaseContainer ancestor = entitySource.GetAncestor();
		if (ancestor)
			return ancestor.GetResourceName();
	}
	
	return string.Empty;	
}


[WorkbenchToolAttribute(name: "Bake MLOD Texture", description: "Bakes MLOD Texture", wbModules: { "WorldEditor" }, shortcut: "Ctrl+B", awesomeFontCode: 0xf7ec)]
class BakeMlodTool : WorldEditorTool
{
	override void OnActivate()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		
		if (!worldEditor.IsPrefabEditMode())
		{
			Print("MLOD Baking is possible only in Edit Mode", LogLevel.WARNING);
			return;
		}
		
		BakeMLODPlugin bakePlugin = BakeMLODPlugin.GetInstance();
		
		//(E): @"scripts/WorkbenchGameCommon/EnfusionBlenderTools/BakeMLOD.c,387": Can't find variable 'SCR_WorldEditorToolHelper'
		//ResourceName openedPrefab = SCR_WorldEditorToolHelper.GetPrefabEditModeResourceName(); 

		
		ResourceName openedPrefab = GetOpenedPrefab(worldEditor, m_API);
		
		if (openedPrefab)
		{
			bakePlugin.StartBake(openedPrefab);
			return;
		}

		Print("Prefab not compatible for MLOD baking", LogLevel.WARNING);
	}
}

class BakeInfoResponse : JsonApiStruct
{
	string Output;

	void BakeInfoResponse()
	{
		RegV("Output");
	}
}

class BakeInfoDialog
{
	[ButtonAttribute("OK")]
	void OkButton(){}
}

class BakeInfo : NetApiHandler
{
	//------------------------------------------------------------------------------------------------
	override JsonApiStruct GetRequest()
	{
		return new BakeInfoRequest();
	}

	//------------------------------------------------------------------------------------------------
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		BakeInfoRequest req = BakeInfoRequest.Cast(request);
		BakeInfoResponse response = new BakeInfoResponse();

		BakeInfoDialog dialog = new BakeInfoDialog();

		Workbench.ScriptDialog("MLOD Baker", req.msg, dialog);

		return response;
	}
}

class OverrideMeshObjectMaterialRequest : JsonApiStruct
{
	string prefabAbsPath;
	string materialAbsPath;
	string textureAbsPath;
	string sourceMaterial;

	void OverrideMeshObjectMaterialRequest()
	{
		RegV("prefabAbsPath");
		RegV("materialAbsPath");
		RegV("textureAbsPath");
		RegV("sourceMaterial");
	}
}

class OverrideMeshObjectMaterialResponse : JsonApiStruct
{
	string Output;

	void OverrideMeshObjectMaterialResponse()
	{
		RegV("Output");
	}
}

static void BakeCreateEmat(string absPath, array<string> properties)
{
	Resource res = BaseContainerTools.CreateContainer("MatPBRBasic");
	BaseContainer cont = res.GetResource().ToBaseContainer();

	ResourceManager resourceManager = Workbench.GetModule(ResourceManager);

	// looping through properties from blender
	for (int i = 0; i < properties.Count(); i+=2)
	{
		if (FilePath.IsAbsolutePath(properties[i+ 1]))
		{
			string importPath = properties[i+ 1];

			//Error
			if (!LoadedProjects.InLoadedProjects(importPath))
				Print("NO TEXTURE!!!!", LogLevel.WARNING);

			MetaFile meta = resourceManager.GetMetaFile(importPath);
			properties[i+ 1] = meta.GetResourceID();
			cont.Set(properties[i], properties[i+ 1]);
		}
		else
		{
			cont.Set(properties[i], properties[i+ 1]);
		}
	}

	BaseContainerTools.SaveContainer(cont, "", absPath);

	resourceManager.RegisterResourceFile(absPath, true);
	resourceManager.WaitForFile(absPath, 3000);

}

static bool ResourceExistsAndIsRegistered(string absPath)
{
	ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
	MetaFile metaFile = resourceManager.GetMetaFile(absPath);
	string dummyResult;

	if (metaFile)
		return Workbench.GetAbsolutePath(metaFile.GetResourceID().GetPath(), dummyResult, true);

	return false;
}

class OverrideMeshObjectMaterial : NetApiHandler
{
	//------------------------------------------------------------------------------------------------
	override JsonApiStruct GetRequest()
	{
		return new OverrideMeshObjectMaterialRequest();
	}

	//------------------------------------------------------------------------------------------------
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

	//------------------------------------------------------------------------------------------------
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		OverrideMeshObjectMaterialRequest req = OverrideMeshObjectMaterialRequest.Cast(request);

		//Get Prefab
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile prefabMeta = resourceManager.GetMetaFile(req.prefabAbsPath);
		Resource resource = Resource.Load(prefabMeta.GetResourceID());
		BaseContainer prefab = resource.GetResource().ToBaseContainer();

		//Get base xob form the prefab, so we could fetch EXT_MLOD material source from it,
		//that we will use to define material overide in the prefab
		BaseContainerList components = prefab.GetObjectArray(EBTContainerFields.components);
		int meshParamID = GetPrefabParamIndex(prefab, EBTContainerFields.meshObject);
		BaseContainer prefabMeshObject = components.Get(meshParamID);

		// TODO: handle errors here when we have more then one matches or we don't have matches
		string sourceMaterialSlot = req.sourceMaterial;

		if (ResourceExistsAndIsRegistered(req.materialAbsPath))
		{
			Print("MLOD BAKER: Material Already Exist. Skipping Creation", LogLevel.NORMAL);
		}
		else
		{
			array<string> materialProps = {"BCRMap", req.textureAbsPath, "MetalnessScale", "0,0", "DisableInteriorProbes", "1"};
			BakeCreateEmat(req.materialAbsPath, materialProps);
		}

		MetaFile materialMeta = resourceManager.GetMetaFile(req.materialAbsPath);
		ResourceName materialResourceName = materialMeta.GetResourceID();

		BaseContainerList prefabMaterials = prefabMeshObject.GetObjectArray("Materials");
    
		if (prefabMaterials.Count() != 0)
		{
			for (int i = 0; i < prefabMaterials.Count(); i++)
			{
				BaseContainer material = prefabMaterials.Get(i);
				ResourceName nameSrc;

				material.Get("SourceMaterial", nameSrc);

				if (nameSrc == sourceMaterialSlot)
				{
					ResourceName nameAssgn;
					material.Get("AssignedMaterial", nameAssgn);
					if (nameAssgn == materialResourceName)
					{
						Print("MLOD BAKER: Material Already Assigned. Skipping Assignment", LogLevel.NORMAL);
						break;
					}

					WorldEditor worldEditorModule = Workbench.GetModule(WorldEditor);
					WorldEditorAPI api = worldEditorModule.GetApi();

					if (!api)
					{
						Workbench.OpenModule(WorldEditor);
						api = worldEditorModule.GetApi();
					}

					ResourceName openedPrefab = GetOpenedPrefab(worldEditorModule, api);
					if (openedPrefab != prefabMeta.GetResourceID())
						worldEditorModule.SetOpenedResource(prefabMeta.GetResourceID());

					bool manageEditAction = !api.IsDoingEditAction();

					if (manageEditAction)
						api.BeginEntityAction();

					api.SetVariableValue(prefab, {ContainerIdPathEntry("MeshObject"), ContainerIdPathEntry("Materials", i)}, "AssignedMaterial", materialResourceName);
					api.SaveEntityTemplate(prefab);

					if (manageEditAction)
						api.EndEntityAction();
					break;
				}
			}
		}
		else
		{
			WorldEditor worldEditorModule = Workbench.GetModule(WorldEditor);
			WorldEditorAPI api = worldEditorModule.GetApi();

			if (!api)
			{
				Workbench.OpenModule(WorldEditor);
				api = worldEditorModule.GetApi();
			}

			ResourceName openedPrefab = GetOpenedPrefab(worldEditorModule, api);
			if (openedPrefab != prefabMeta.GetResourceID())
				worldEditorModule.SetOpenedResource(prefabMeta.GetResourceID());

			bool manageEditAction = !api.IsDoingEditAction();

			if (manageEditAction)
				api.BeginEntityAction();

			api.CreateObjectArrayVariableMember(prefab, {ContainerIdPathEntry("MeshObject")}, "Materials", "MaterialAssignClass", 0);
			api.SetVariableValue(prefab, {ContainerIdPathEntry("MeshObject"), ContainerIdPathEntry("Materials", 0)}, "SourceMaterial", sourceMaterialSlot);
			api.SetVariableValue(prefab, {ContainerIdPathEntry("MeshObject"), ContainerIdPathEntry("Materials", 0)}, "AssignedMaterial", materialResourceName);

			if (manageEditAction)
				api.EndEntityAction();

			api.SaveEntityTemplate(prefab);

		}

		OverrideMeshObjectMaterialResponse response = new OverrideMeshObjectMaterialResponse();

		return response;
	}
}
#endif
