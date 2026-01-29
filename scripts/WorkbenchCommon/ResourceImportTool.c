[WorkbenchPluginAttribute("Model Import", "Model Import Helper", "", "", {"ResourceManager"})]
class ResourceImportPlugin: ResourceManagerPlugin
{
	[Attribute("true", UIWidgets.CheckBox)]
	bool Enabled;

	//--------------------------------------------------------------------
	void ResourceImportPlugin()
	{
	}

	//--------------------------------------------------------------------
	override string OnGetMaterialGenerateDir(string absModelPath)
	{
		return absModelPath + "/Data";
	}
	
	//--------------------------------------------------------------------
	override void OnRegisterResource(string absFileName, BaseContainer metaFile)
	{
		BaseContainer conf = metaFile.GetObjectArray("Configurations")[0];
		if (!Enabled)
			return;

		string className = conf.GetClassName();

		if (ResourceProcessorPlugin.IsMeshObject(className))
		{
			ResourceProcessorPlugin.FixMeshObjectMetaFile(metaFile, absFileName);
		}
		else if (ResourceProcessorPlugin.IsSound(className))
		{
			ResourceProcessorPlugin.FixSoundMetaFile(metaFile, absFileName);
		}
		else if (ResourceProcessorPlugin.IsParticleEffect(className))
		{
			ResourceProcessorPlugin.FixParticleEffectMetaFile(metaFile, absFileName);
		}
	}

	override void Configure()
	{
		Workbench.ScriptDialog("Configure Texture Import Plugin", "", this);
	}

	[ButtonAttribute("OK")]
	void OkButton() {}
}

