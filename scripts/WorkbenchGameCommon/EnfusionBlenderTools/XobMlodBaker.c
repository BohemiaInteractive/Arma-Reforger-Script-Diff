class BakeMLODResult : JsonApiStruct
{
	string bcr;
	string nmo;
}

class BakeXOBMlodRequest : JsonApiStruct
{
	ResourceName xobResourceName;
	ref array<string> renderPasses;
	int samples;
	int padding;
	float offset;
	int resolution;
	ref array<float> blockerColor;
	float blockerRoughness;
	float blockerMetalness;
	bool usePortalAsBlockers;

	//------------------------------------------------------------------------------------------------
	void BakeXOBMlodRequest()
	{
		RegAll();
	}
}

//------------------------------------------------------------------------------------------------
// RestAPI
BakeMLODResult BakeXOBMlod(notnull const BlenderRestAPI connection, notnull BakeXOBMlodRequest request)
{
	auto result = RestAPIHelper<BakeMLODResult>.Post(connection, "/bake_xob_mlod", request);

	if (result)
		return result;

	return null;
}

class MlodBakeJob
{
	protected ref BakeXOBMlodRequest m_Request;
	protected ResourceName m_TargetMaterial;

	void MlodBakeJob(BakeXOBMlodRequest request, ResourceName targetMaterial)
	{
		m_Request = request;
		m_TargetMaterial = targetMaterial;
	}

	protected static string SetFilePathExtension(string filePath, string extension)
	{
		return FilePath.StripExtension(filePath) + extension;
	}

	protected static string CreateTexturePathFromMaterialAbsolutePath(string materialPath, string textureType)
	{
		string postfix = string.Format("_%1", textureType);
		return SetFilePathExtension(string.Format("%1%2", FilePath.StripExtension(materialPath), postfix), ".tif");
	}

	protected static string GetTextureSavePath(string textureType, ResourceName targetMaterial)
	{
		string textureSlotName = string.Format("%1%2", textureType, "Map");
		ResourceName assignedTextureResource = ResourceUtils.GetTextureResourceFromMaterialSlot(targetMaterial, textureSlotName);

		if (assignedTextureResource == string.Empty)
		{
			string materialAbsPath = ResourceUtils.GetAbsolutePathForResourceName(targetMaterial);
			if (materialAbsPath == string.Empty)
				return string.Empty;

			return CreateTexturePathFromMaterialAbsolutePath(materialAbsPath, textureType);
		}

		string assignedTextureAbsPath = ResourceUtils.GetAbsolutePathForResourceName(assignedTextureResource);

		if (assignedTextureAbsPath == string.Empty)
			return string.Empty;

		return SetFilePathExtension(assignedTextureAbsPath, ".tif");
	}

	protected static ResourceName SaveBakedTexture(string sourceTexturePath, string textureType, ResourceName targetMaterial)
	{
		string textureSavePath = GetTextureSavePath(textureType, targetMaterial);
		if (textureSavePath == string.Empty)
		{
			Print(string.Format("Could not resolve a save path for the %1 texture of material '%2'.", textureType, targetMaterial.GetPath()), LogLevel.ERROR);
			return string.Empty;
		}

		if (!ResourceUtils.RegisterTextureResource(sourceTexturePath, textureSavePath))
		{
			Print(string.Format("Could not register the baked %1 texture as '%2'.", textureType, textureSavePath), LogLevel.ERROR);
			return string.Empty;
		}

		if (!ResourceUtils.WaitForResourceFile(SetFilePathExtension(textureSavePath, ".edds")))
			return string.Empty;

		Print(string.Format("Saved baked %1 texture to '%2'.", textureType, textureSavePath), LogLevel.NORMAL);
		return ResourceUtils.GetResourceNameForAbsolutePath(textureSavePath);
	}

	//! Runs this threaded.
	void Run()
	{
		auto connection = BlenderRestAPI.Create();
		if (!connection)
		{
			Print("Unable to create Blender REST connection.", LogLevel.ERROR);
			return;
		}

		auto bakeResult = BakeXOBMlod(connection, m_Request);
		connection.Destroy();

		if (!bakeResult || (bakeResult.bcr == string.Empty && bakeResult.nmo == string.Empty))
		{
			Print("Failed to bake MLOD through Blender REST API.", LogLevel.ERROR);
			return;
		}

		ResourceName bcrTextureResource;
		ResourceName nmoTextureResource;

        if (bakeResult.bcr != string.Empty)
			bcrTextureResource = SaveBakedTexture(bakeResult.bcr, "BCR", m_TargetMaterial);
            
		if (bakeResult.nmo != string.Empty)
			nmoTextureResource = SaveBakedTexture(bakeResult.nmo, "NMO", m_TargetMaterial);
			
		
		Resource resource = BaseContainerTools.LoadContainer(m_TargetMaterial);
		if (!resource || !resource.IsValid())
		{
			Print(string.Format("Could not load MLOD material '%1'", m_TargetMaterial.GetPath()), LogLevel.ERROR);
			return;
		}

		BaseContainer materialContainer = resource.GetResource().ToBaseContainer();
		if (!materialContainer)
		{
			Print(string.Format("Could not get container for MLOD material '%1'", m_TargetMaterial.GetPath()), LogLevel.ERROR);
			return;
		}

		bool materialChanged;
		if (bcrTextureResource != string.Empty && materialContainer.Set("BCRMap", bcrTextureResource))
			materialChanged = true;

		if (nmoTextureResource != string.Empty && materialContainer.Set("NMOMap", nmoTextureResource))
			materialChanged = true;

		if (materialChanged)
			BaseContainerTools.SaveContainer(materialContainer, m_TargetMaterial);
	}
}

class MLODBakeSettingsDialog
{
	[Attribute("1", UIWidgets.ComboBox, "Square bake resolution. Used for both width and height.", "", enums : ParamEnumTextureSize.FromEnum(), category : "Bake")] int m_iResolution = 1;

	[Attribute("16", UIWidgets.EditBox, "", category : "Bake")] int m_iSamples = 16;

	[Attribute("512", UIWidgets.EditBox, "", category : "Bake")] int m_iPadding = 512;

	[Attribute("2.0", UIWidgets.EditBox, "", category : "Bake")] float m_fOffset = 2.0;

	[Attribute("0 0 0 1", UIWidgets.ColorPicker, "", category : "Blockers")] ref Color m_BlockerColor = new Color(0, 0, 0, 1);

	[Attribute("0.0", UIWidgets.EditBox, "", category : "Blockers")] float m_fBlockerMetalness = 0;

	[Attribute("1.0", UIWidgets.EditBox, "", category : "Blockers")] float m_fBlockerRoughness = 1;

	[Attribute("false", UIWidgets.CheckBox, "", category : "Blockers")] bool m_bUsePortalsAsBlockers = false;

	[Attribute("false", UIWidgets.CheckBox, "", category : "Render Passes")] bool m_bNMO = false;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Bake")] bool BakeButton()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")] bool CancelButton()
	{
		return false;
	}
}

[WorkbenchPluginAttribute(name : "EBT: Bake XOB MLOD", wbModules : {"ResourceManager"}, resourceTypes : {"xob"}, awesomeFontCode : 0xf7ec)] 
class NewMlodBakerPlugin : WorkbenchPlugin
{
	protected ref MlodBakeJob m_ActiveJob;
	protected static ref MLODBakeSettingsDialog s_SettingsDialog = new MLODBakeSettingsDialog();

	protected static array<string> ResolveRenderPasses(MLODBakeSettingsDialog dialog)
	{
		array<string> passes = {};
		passes.Insert("BCR"); // BCR is always included
		if (dialog.m_bNMO)
			passes.Insert("NMO");

		return passes;
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName GetMlodMaterial(ResourceName xobResourceName)
	{
		MetaFile xobMeta = ResourceUtils.GetMetaFileForResourceName(xobResourceName);

		ResourceName assignedMaterialResourceName;
		if (!ResourceUtils.GetMlodMaterialResourceFromXOBMeta(xobMeta, assignedMaterialResourceName))
		{
			Print(string.Format("%1 has no assigned MLOD material", xobResourceName), LogLevel.ERROR);
			return string.Empty;
		}

		MetaFile materialMeta = ResourceUtils.GetMetaFileForResourceName(assignedMaterialResourceName);
		if(!materialMeta)
		{
			Print(string.Format("%1 has no MLOD material; there is nothing to bake into.", xobResourceName), LogLevel.ERROR);
			return string.Empty;
		}

		string materialClassName = ResourceUtils.GetMaterialClassName(assignedMaterialResourceName);
		if (materialClassName != "MatPBRBasic")
		{
			Print(string.Format("MLOD material '%1' of %2 is a %3; only MatPBRBasic materials can be baked into.", assignedMaterialResourceName.GetPath(), xobResourceName, materialClassName), LogLevel.ERROR);
			return string.Empty;
		}

		return assignedMaterialResourceName;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ValidateXOBResource(ResourceName xobResourceName)
	{
		MetaFile meta = ResourceUtils.GetMetaFileForResourceName(xobResourceName);
		if (!meta)
		{
			Print(string.Format("Could not load the MetaFile of %1.", xobResourceName), LogLevel.ERROR);
			return false;
		}

		string fbxSourcePath;
		if (!Workbench.GetAbsolutePath(meta.GetSourceFilePath(), fbxSourcePath, true))
		{
			Print(string.Format("%1 has no source FBX file to bake from.", xobResourceName), LogLevel.ERROR);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected BakeXOBMlodRequest CreateBakeRequest(ResourceName xobResourceName, MLODBakeSettingsDialog dialog)
	{
		if (!Workbench.ScriptDialog("Bake MLOD", "Bake square texture from the last two LOD groups.", dialog))
			return null;

		BakeXOBMlodRequest request = new BakeXOBMlodRequest();
		request.xobResourceName = xobResourceName;
		request.renderPasses = ResolveRenderPasses(dialog);;
		request.samples = dialog.m_iSamples;
		request.padding = dialog.m_iPadding;
		request.offset = dialog.m_fOffset;
		request.resolution = TEXTURE_SIZES[dialog.m_iResolution];
		request.blockerColor = {dialog.m_BlockerColor.R(), dialog.m_BlockerColor.G(), dialog.m_BlockerColor.B(), dialog.m_BlockerColor.A()};
		request.blockerMetalness = dialog.m_fBlockerMetalness;
		request.blockerRoughness = dialog.m_fBlockerRoughness;
		request.usePortalAsBlockers = dialog.m_bUsePortalsAsBlockers;

		return request;
	}

	//------------------------------------------------------------------------------------------------
	override void OnResourceContextMenu(notnull array<ResourceName> resources)
	{
		string pathToExecutable;
		if (!EBTConfigPlugin.GetBlenderPathOrWarn(pathToExecutable))
			return;

		if(resources.IsEmpty())
		{
			Print("No selected resources", LogLevel.WARNING);
			return;
		}

		ResourceName xobResourceName = resources[0];

		if(!ValidateXOBResource(xobResourceName))
			return;

		ResourceName targetMaterial = GetMlodMaterial(xobResourceName);

		if(targetMaterial == string.Empty)
			return;

		auto request = CreateBakeRequest(xobResourceName, s_SettingsDialog);

		if(!request)
			return;

		m_ActiveJob = new MlodBakeJob(request, targetMaterial); // hold the ref, otherwise job is released at end of scope

		thread m_ActiveJob.Run();
	}
}
