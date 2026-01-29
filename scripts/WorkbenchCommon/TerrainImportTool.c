[WorkbenchPluginAttribute("Terrain Import", "Terrain Import Helper", "", "", {"ResourceManager"})]
class TerrainImportPlugin: ResourceManagerPlugin
{
	[Attribute("true", UIWidgets.CheckBox)]
	bool Enabled;
	
	//--------------------------------------------------------------------
	override void OnRegisterResource(string absFileName, BaseContainer metaFile)
	{
		BaseContainer conf = metaFile.GetObjectArray("Configurations")[0];
		if (!Enabled || conf.GetClassName() != "TerrainResourceClass")
			return;
		
		BaseContainerList configurations = metaFile.GetObjectArray("Configurations");
		if (!configurations)
			return;
			
		map<string, ref array<ResourceName>> textureConfigs = new map<string, ref array<ResourceName>>;
		textureConfigs.Insert("PC", {
		"{A59F335F96C4442F}configs/ResourceTypes/PC/TextureTerrainLayer.conf",
		"{835E083C88C3D9C3}configs/ResourceTypes/PC/TextureTerrainNormal.conf",
		"{CBA1266690ABD336}configs/ResourceTypes/PC/TextureTerrainSuper.conf"
		});
		
		textureConfigs.Insert("PS4", {
		"{4C431036F57D4D58}configs/ResourceTypes/PS4/TextureTerrainLayer.conf",
		"{6F6E5A1432381FD2}configs/ResourceTypes/PS4/TextureTerrainNormal.conf",
		"{227D050FF312DA41}configs/ResourceTypes/PS4/TextureTerrainSuper.conf"
		});
		
		textureConfigs.Insert("PS5", {
		"{BA3AFE83BEFD6794}configs/ResourceTypes/PS5/TextureTerrainLayer.conf",
		"{5CEA2418C9F889F8}configs/ResourceTypes/PS5/TextureTerrainNormal.conf",
		"{D404EBBAB892F08D}configs/ResourceTypes/PS5/TextureTerrainSuper.conf"
		});
		
		textureConfigs.Insert("XBOX_ONE", {
		"{9843D16A29542D5A}configs/ResourceTypes/XBOX_ONE/TextureTerrainLayer.conf",
		"{6C5EFFA712A52100}configs/ResourceTypes/XBOX_ONE/TextureTerrainNormal.conf",
		"{F67DC4532F3BBA43}configs/ResourceTypes/XBOX_ONE/TextureTerrainSuper.conf"
		});
		
		textureConfigs.Insert("XBOX_SERIES", {
		"{2071F9D39E062268}configs/ResourceTypes/XBOX_SERIES/TextureTerrainLayer.conf",
		"{90ADE65A903F3042}configs/ResourceTypes/XBOX_SERIES/TextureTerrainNormal.conf",
		"{4E4FECEA9869B571}configs/ResourceTypes/XBOX_SERIES/TextureTerrainSuper.conf"
		});
		
		textureConfigs.Insert("HEADLESS", {
		"{3B38AD285230D19F}configs/ResourceTypes/HEADLESS/TextureTerrainLayer.conf",
		"{EAB38BDFD096C0A6}configs/ResourceTypes/HEADLESS/TextureTerrainNormal.conf",
		"{5506B811545F4686}configs/ResourceTypes/HEADLESS/TextureTerrainSuper.conf"
		});
		
		for (int c = 0; c < configurations.Count(); c++)
		{
			BaseContainer cfg = configurations.Get(c);
			string cfgName = cfg.GetName();
			
			array<ResourceName> texConfig;
			if (textureConfigs.Find(cfgName, texConfig))
			{
				SetConfig(cfg, "LayerSource", texConfig[0]);
				SetConfig(cfg, "NormalSource", texConfig[1]);
				SetConfig(cfg, "SuperSource", texConfig[2]);
			}
		}
	}
	
	void SetConfig(BaseContainer cfg, string field, ResourceName configPath)
	{
		Resource src = BaseContainerTools.CreateContainer("SourceTextureClass");
		src.GetResource().ToBaseContainer().SetAncestor(configPath);
			
		cfg.SetObject(field, src.GetResource().ToBaseContainer());
	}
	
	override void Configure()
	{
		Workbench.ScriptDialog("Configure Texture Import Plugin", "", this);
	}

	[ButtonAttribute("OK")]
	void OkButton() {}
}
