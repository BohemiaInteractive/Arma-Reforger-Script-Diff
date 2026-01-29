[WorkbenchPluginAttribute("Batch resource processor", "Perform simple checks and fixes on resource files", "", "", {"ResourceManager"},"",0xf15b)]
class ResourceProcessorPlugin: WorkbenchPlugin
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Types (file extensions) to process")]
	ref array<string> FileTypes;

	[Attribute("", UIWidgets.FileNamePicker, "Check only textures whose path starts with given filter string.", params:"folders")]
	string PathStartsWith;

	[Attribute("", UIWidgets.EditBox, "Check only textures whose path contains given filter string.")]
	string PathContains;

	[Attribute("", UIWidgets.EditBox, "Check only textures whose path ends with given filter string.")]
	string PathEndsWith;
	
	[Attribute("true", UIWidgets.CheckBox, "Change configuration inheritance etc")]
	bool FixMetaFile;
	
	[Attribute("true", UIWidgets.CheckBox, "Resaves metafile even if it's without changes")]
	bool ForceResaveMetaFile;
	
	[Attribute("true", UIWidgets.CheckBox, "Report missing meta-file")]
	bool ReportMissingMetaFile;
	
	[Attribute("true", UIWidgets.CheckBox, "Report missing configurations")]
	bool ReportMissingConfigurations;
	
	[Attribute("true", UIWidgets.CheckBox, "Report custom property values")]
	bool ReportCustomPropertyValues;
	
	[Attribute("false", UIWidgets.CheckBox, "Fix custom property values (sets all PC custom changes to all other platforms)")]
	bool FixCustomPropertyValues;
	
	ref array<string> m_Resources = new array<string>;
	
	//----------------------------------------------------------------------------------------------
	void ResourceProcessorPlugin()
	{
	}

	//----------------------------------------------------------------------------------------------
	[ButtonAttribute("Run", true)]
	bool OK()
	{
		return true;
	}

	//----------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	bool Cancel()
	{
		return false;
	}

	
	static const string MeshObjectCommon = "Common";
	
	
	//----------------------------------------------------------------------------------------------
	bool TestAgainstFilter(string resource)
	{
		int resourceLength = resource.Length();

		int prefixLength = PathStartsWith.Length();
		if (resourceLength < prefixLength)
			return false;

		int suffixLength = PathEndsWith.Length();
		if (resourceLength < suffixLength)
			return false;

		int subLength = PathContains.Length();
		if (resourceLength < subLength)
			return false;

		resource.ToLower();
		if (!resource.StartsWith(PathStartsWith))
			return false;

		if (!resource.EndsWith(PathEndsWith))
			return false;

		if (!resource.Contains(PathContains))
			return false;

		return true;
	}
	
	//----------------------------------------------------------------------------------------------
	void Find(ResourceName resName, string filePath)
	{
		if (TestAgainstFilter(filePath))
			m_Resources.Insert(filePath);
	}
	
	//----------------------------------------------------------------------------------------------
	bool CheckContainersVar(string resource, notnull array<string> platformPrefixes, notnull array<BaseContainer> containers, string varName, DataVarType type)
	{
		BaseContainer pc = containers[0];
		bool isSetOnPC = pc.IsVariableSetDirectly(varName);
		bool bRes = true;
		
		if (type == DataVarType.OBJECT)
		{
			array<BaseContainer> subContainers	= {};
			array<string> subPlatformPrefixes = {};
			
			for (int iPlatform = 0; iPlatform < containers.Count(); iPlatform++)
			{
				BaseContainer platform = containers[iPlatform];
				subContainers.Insert(platform.GetObject(varName));
				subPlatformPrefixes.Insert(platformPrefixes[iPlatform] + "." + varName);
			}
			
			BaseContainer pcSubContainer = subContainers[0];
			if (pcSubContainer)
			{
				int nVars = pcSubContainer.GetNumVars();
				for (int iVar = 0; iVar < nVars; iVar++)
				{
					string subVarName = pcSubContainer.GetVarName(iVar);
										
					if (!CheckContainersVar(resource, subPlatformPrefixes, subContainers, subVarName, pcSubContainer.GetDataVarType(iVar)))
					{
						bRes = false;
					}
				}
				return bRes;
			}
		}

		for (int iPlatform = 1; iPlatform < containers.Count(); iPlatform++)
		{
			BaseContainer platform = containers[iPlatform];
			bool isSet = platform.IsVariableSetDirectly(varName);
			bool isInherited = false;
			
			if (platformPrefixes[iPlatform].StartsWith("HEADLESS"))
				continue;
			
			if (platform.GetAncestor() && platform.GetAncestor().GetName() == pc.GetName())
			{
				isInherited = true;
			}
			
			//--
			
			if (isSetOnPC)
			{
				if (isInherited)
				{
					if (isSet)
					{
						string valPC;
						string valPlatform;
						
						if (type != DataVarType.OBJECT_ARRAY)
						{
							pc.Get(varName, valPC);						
							pc.Get(varName, valPlatform);
						}
						
						if (valPC != valPlatform)
						{
							if (FixCustomPropertyValues)
							{
								FixIssue(resource, pc, platform, platformPrefixes[iPlatform], varName);
							}
							else
							{
								ReportIssue(resource, "Property inherited from PC, but overridden on platform: " + platformPrefixes[iPlatform] + "." + varName);
							}
							bRes = false;
						}
					}
				}
				else
				{
					if (!isSet)
					{
						if (FixCustomPropertyValues)
						{
							FixIssue(resource, pc, platform, platformPrefixes[iPlatform], varName);
						}
						else
						{
							ReportIssue(resource, "Property set on PC, but not set on platform: " + platformPrefixes[iPlatform] + "." + varName);
						}
						bRes = false;
					}					
				}
			} 
			else
			{
				if (isSet)
				{
					if (FixCustomPropertyValues)
					{
						FixIssue(resource, pc, platform, platformPrefixes[iPlatform], varName);
					}
					else
					{
						ReportIssue(resource, "Property not set on PC, but set on plarform: " + platformPrefixes[iPlatform] + "." + varName);
					}
					bRes = false;
				}
			}
		}
		
		return bRes;
	}
	
	//----------------------------------------------------------------------------------------------
	bool Execute()
	{
		PathStartsWith.ToLower();
		PathContains.ToLower();
		PathEndsWith.ToLower();
		
		m_Resources.Clear();
		
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = FileTypes;
		ResourceDatabase.SearchResources(filter, Find);
		
		m_Resources.Sort();

		bool bRes = true;
		ResourceManager rb = Workbench.GetModule(ResourceManager);
		WBProgressDialog progress = new WBProgressDialog("Processing...", rb);
		
		Print("Batch resource processor - BEGIN");

		float count = m_Resources.Count();

		foreach (int resourceIdx, string resource : m_Resources)
		{
			progress.SetProgress(resourceIdx / count);
				
			MetaFile meta = rb.GetMetaFile(resource);
			if (!meta)
			{
				if (ReportMissingMetaFile)
				{
					ReportIssue(resource, "meta-file is missing");
					bRes = false;
				}
				continue;
			}

			BaseContainerList configurations = meta.GetObjectArray("Configurations");
			if (!configurations)
			{
				if (ReportMissingConfigurations)
				{
					ReportIssue(resource, "meta-file is missing 'Configurations' property");
					bRes = false;
				}
				meta.Release();
				continue;
			}

			bool anyChangeInMetaFile = ForceResaveMetaFile;
			
			if (ReportCustomPropertyValues || FixCustomPropertyValues)
			{
				int nPlatforms = configurations.Count();
				
				ResourceName pcConfigName = configurations[0].GetResourceName();
				array<string> pcConfigVarsChanged = {};
				BaseContainer pcPlatformConf = configurations.Get(0);
				
				//---
				array<BaseContainer> containers = {};
				array<string> platformPrefixes = {};
				
				for (int iPlatform = 0; iPlatform < nPlatforms; iPlatform++)
				{
					BaseContainer platformConf = configurations.Get(iPlatform);
					containers.Insert(platformConf);
					platformPrefixes.Insert(platformConf.GetName());
				}
				
				int nVars = pcPlatformConf.GetNumVars();
				for (int iVar = 0; iVar < nVars; iVar++)
				{
					string varName = pcPlatformConf.GetVarName(iVar);
										
					if (!CheckContainersVar(resource, platformPrefixes, containers, varName, pcPlatformConf.GetDataVarType(iVar)))
					{
						bRes = false;
						if (FixCustomPropertyValues)
							anyChangeInMetaFile = true;
					}
				}
			}
			
			if(FixMetaFile)
			{
				BaseContainer conf = configurations.Get(0);
				string className = conf.GetClassName();
				if( IsMeshObject(className) )
				{
					if( FixMeshObjectMetaFile(meta, resource) )
					{
						bRes = false;
						anyChangeInMetaFile = true;
					}
				}
				else if( IsSound(className) )
				{
					if( FixSoundMetaFile(meta, resource) )
						anyChangeInMetaFile = true;
				}
				else if( IsParticleEffect(className) )
				{
					if( FixParticleEffectMetaFile(meta, resource) )
						anyChangeInMetaFile = true;
				}
			}

			if(anyChangeInMetaFile )
			{
//				Print(resource);
				meta.Save();
			}
			meta.Release();
		}
		
		Print("Batch resource processor - END");
		
		return bRes;
	}
	
	//----------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("Batch resource processor", "", this))
			return;
		
		Execute();
	}
	
	//----------------------------------------------------------------------------------------------
	// CLI: -FileTypes="xob,edds" PathStartsWith="$ArmaReforger:Assets/Structures/Houses"
	override void RunCommandline()
	{
		FixCustomPropertyValues = false;
		FixMetaFile = false;
		ForceResaveMetaFile = false;
		
		ReportMissingMetaFile = true;
		ReportMissingConfigurations = true;
		ReportCustomPropertyValues = true;
		
		FileTypes.Clear();
		PathStartsWith = string.Empty;
		PathContains = string.Empty;
		PathEndsWith = string.Empty;
		
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		string cli;
		
		rm.GetCmdLine("-FileTypes", cli);
		cli.Split(",", FileTypes, true);
		
		rm.GetCmdLine("-PathStartsWith", PathStartsWith);
		rm.GetCmdLine("-PathContains", PathContains);
		rm.GetCmdLine("-PathEndsWith", PathEndsWith);
		
		Print("BatchMeshObjectProcessorPlugin commandline:");
		Print(PathStartsWith);
		Print(PathContains);
		Print(PathEndsWith);	
		FileTypes.Debug();
		
		if (Execute())
			Workbench.Exit(0);
		else
			Workbench.Exit(-1);
	}
	
	//----------------------------------------------------------------------------------------------
	static void ReportIssue(string resource, string issue)
	{
		string msg = string.Format("@\"%1\" : %2", resource, issue);
		Print(string.ToString(msg, false, false, false), LogLevel.ERROR);
	}
	
	//----------------------------------------------------------------------------------------------
	static void FixIssue(string resource, BaseContainer pc, BaseContainer platform, string prefix, string varName)
	{
		string val;
		pc.Get(varName, val);
		platform.Set(varName, val);
		ReportIssue(resource, "Property fixed: " + prefix + "." + varName);
	}

	//----------------------------------------------------------------------------------------------
	static bool FixMeshObjectMetaFile(MetaFile meta, string absFileName)
	{
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		if(!configurations)
			return false;

		bool metaModified = false;
		int pcIdx = -1;
		for(int c = 0; c < configurations.Count(); c++)
		{
			BaseContainer cfg = configurations.Get(c);

			string cfgName = cfg.GetName();
			if (cfgName == "PC")
				pcIdx = c;

			//ensure that object which we want set ancestor to does exist. Create new instance if it's missing
			bool IsSet = cfg.IsVariableSetDirectly(MeshObjectCommon);
			if( !IsSet )
			{
				Resource res = BaseContainerTools.CreateContainer("TXOCommonClass");
				BaseResourceObject root = res.GetResource();
				BaseContainer cont = root.ToBaseContainer();
				cfg.SetObject(MeshObjectCommon, cont);
			}

			//get an object which must be inherited from base configs
			BaseContainer commonObj = cfg.GetObject(MeshObjectCommon);
			if(!commonObj)
			{
				ReportIssue(absFileName, "Cannot set configuration ancestor (" + cfgName + ")" );
				continue;
			}

			ResourceName ancestor = "";

			switch(cfgName)
			{
				case "PC":
					ancestor = "{0877E7C4BB2B2C9A}configs/ResourceTypes/PC/MeshObjectCommon.conf";
					break;
				case "XBOX_ONE":
					ancestor = "{9D5B6207F7628CE2}configs/ResourceTypes/XBOX_ONE/MeshObjectCommon.conf";
					break;
				case "XBOX_SERIES":
					ancestor = "{DD9F02115C764647}configs/ResourceTypes/XBOX_SERIES/MeshObjectCommon.conf";
					break;
				case "PS4":
					ancestor = "{53EC476BC921D99A}configs/ResourceTypes/PS4/MeshObjectCommon.conf";
					break;
				case "PS5":
					ancestor = "{F62AA8E7B8EA9D26}configs/ResourceTypes/PS5/MeshObjectCommon.conf";
					break;
				case "HEADLESS":
					ancestor = "{3A5B3356978039E8}configs/ResourceTypes/HEADLESS/MeshObjectCommon.conf";
					break;
			}

			if(ancestor != "")
			{
				commonObj.SetAncestor(ancestor);
				metaModified = true;
			}
		}

		if (pcIdx >= 0)
		{
			BaseContainer confPc_ = configurations[pcIdx];
			BaseContainer commonPc = confPc_.GetObject(MeshObjectCommon);
			for (int iConf = 0, countConf = configurations.Count(); iConf < countConf; iConf++)
			{
				if (iConf == pcIdx)
					continue;

				BaseContainer confDerived_ = configurations.Get(iConf);
				BaseContainer commonDerived = confDerived_.GetObject(MeshObjectCommon);
				if (!commonDerived)
					continue;

				for (int iVar = 0, countVar = commonPc.GetNumVars(); iVar < countVar; iVar++)
				{
					string namePc = commonPc.GetVarName(iVar);
					string nameDerived = commonDerived.GetVarName(iVar);
					if (namePc != nameDerived)
						continue;

					DataVarType varType = commonPc.GetDataVarType(iVar);
					bool isSetDirectlyPc = commonPc.IsVariableSetDirectly(namePc);
					bool isSetDirectlyDerived = commonDerived.IsVariableSetDirectly(nameDerived);
					if (isSetDirectlyPc)
					{
						if (varType == DataVarType.SCALAR)
						{
							float propValPc, propValDerived;
							if (!commonPc.Get(namePc, propValPc))
								continue;

							if (!commonDerived.Get(nameDerived, propValDerived))
								continue;

							if (propValPc != propValDerived)
							{
								metaModified = true;
								commonDerived.Set(nameDerived, propValPc);
							}
						}
						else
						{
							int propValPc, propValDerived;
							if (!commonPc.Get(namePc, propValPc))
								continue;

							if (!commonDerived.Get(nameDerived, propValDerived))
								continue;

							if (propValPc != propValDerived)
							{
								metaModified = true;
								commonDerived.Set(nameDerived, propValPc);
							}
						}
					}
					else if (isSetDirectlyDerived)
					{
						metaModified = true;
						commonDerived.ClearVariable(nameDerived);
					}
				}
			}
		}

		return metaModified;
	}

	//----------------------------------------------------------------------------------------------
	static bool FixSoundMetaFile(MetaFile meta, string absFileName)
	{
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		if(!configurations)
			return false;
			
		bool anyChangeInMetaFile = false;
		
		for(int c = 0; c < configurations.Count(); c++)
		{
			BaseContainer cfg = configurations.Get(c);
			
			string cfgName = cfg.GetName();
			ResourceName ancestor = "";
			
			switch(cfgName)
			{
				case "PC":
					ancestor = "{F72B05D02F3F135E}configs/ResourceTypes/PC/Sound.conf";
					break;
				case "XBOX_ONE":
					ancestor = "{A0776163143143AC}configs/ResourceTypes/XBOX_ONE/Sound.conf";
					break;
				case "XBOX_SERIES":
					ancestor = "{382981CA1FC1C8DB}configs/ResourceTypes/XBOX_SERIES/Sound.conf";
					break;
				case "PS4":
					ancestor = "{89EB939911B4C093}configs/ResourceTypes/PS4/Sound.conf";
					break;
				case "PS5":
					ancestor = "{A584A3B556D4A981}configs/ResourceTypes/PS5/Sound.conf";
					break;
				case "HEADLESS":
					ancestor = "{D63592CA950AF3A8}configs/ResourceTypes/HEADLESS/Sound.conf";
					break;
			}
			
			if(ancestor != "")
			{
				cfg.SetAncestor(ancestor);						
				anyChangeInMetaFile = true;
			}
		}
		
		return anyChangeInMetaFile;
	}

	//----------------------------------------------------------------------------------------------
	static bool FixParticleEffectMetaFile(MetaFile meta, string absFileName)
	{
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		if(!configurations)
			return false;
			
		bool anyChangeInMetaFile = false;
		
		for(int c = 0; c < configurations.Count(); c++)
		{
			BaseContainer cfg = configurations.Get(c);
			
			string cfgName = cfg.GetName();
			ResourceName ancestor = "";
			
			switch(cfgName)
			{
				case "PC":
					ancestor = "{4B4500E061CCD624}configs/ResourceTypes/PC/ParticleEffect.conf";
					break;
				case "XBOX_ONE":
					ancestor = "{73FE8562A95EBA36}configs/ResourceTypes/XBOX_ONE/ParticleEffect.conf";
					break;
				case "XBOX_SERIES":
					ancestor = "{DC1673718B31CF7E}configs/ResourceTypes/XBOX_SERIES/ParticleEffect.conf";
					break;
				case "PS4":
					ancestor = "{B94602D6C3160C98}configs/ResourceTypes/PS4/ParticleEffect.conf";
					break;
				case "PS5":
					ancestor = "{E8F9F77ECEB75947}configs/ResourceTypes/PS5/ParticleEffect.conf";
					break;
				case "HEADLESS":
					ancestor = "{EED8908A9A55ED1C}configs/ResourceTypes/HEADLESS/ParticleEffect.conf";
					break;
			}
			
			if(ancestor != "")
			{
				cfg.SetAncestor(ancestor);						
				anyChangeInMetaFile = true;
			}
		}
		
		return anyChangeInMetaFile;
	}

	//--------------------------------------------------------------------
	static bool IsMeshObject(string className)
	{
		return	className == "FBXResourceClass" || className == "TXOResourceClass";
	}
	
	//--------------------------------------------------------------------
	static bool IsSound(string className)
	{
		 return className == "WAVResourceClass";
	}
	
	//--------------------------------------------------------------------
	static bool IsParticleEffect(string className)
	{
		 return className == "PTCResourceClass";
	}
}