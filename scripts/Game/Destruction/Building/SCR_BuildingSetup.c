#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_BuildingSetup
{
	[Attribute("1", UIWidgets.EditBox, "", "1 500 1")]
	int m_iNumRegions;
	[Attribute("100", UIWidgets.Slider, "Maximum drop of falling debris (in m)", "1 500 1")]
	float m_fLargeDebrisDropMax;
	[Attribute("0", UIWidgets.CheckBox, "If true, no debris particle effects and pieces will be spawned")]
	bool m_bNoDebrisFX;
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Model to use when the building is undamaged")]
	ResourceName m_ModelUndamaged;
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Model to use when the building is destroyed")]
	ResourceName m_ModelDestroyed;
	[Attribute("", UIWidgets.EditBox, "Model base path to use for the damaged region parts (automatically adds index to the end, eg: m_ModelDamagedPartBase00)")]
	string m_ModelDamagedPartBase;
	[Attribute("", UIWidgets.EditBox, "Model base path to use for the debris in the intersection between two region parts, when visible (m_ModelRegionIntersectDebrisBase + index + m_ModelRegionIntersectDebrisPrefix + otherIndex, eg: models/structure01/house_region00_rubble01)")]
	string m_ModelRegionIntersectDebrisBase;
	[Attribute("_rubble", UIWidgets.EditBox, "Prefix to add to the base path to use for the debris in the intersection between two region parts, when visible (m_ModelRegionIntersectDebrisBase + index + m_ModelRegionIntersectDebrisPrefix + otherIndex, eg: models/structure01/house_region00_rubble01)")]
	string m_ModelRegionIntersectDebrisPrefix;
	
	[Attribute("", UIWidgets.Object, "Default definition settings for all regions")]
	ref SCR_BuildingRegionSetup m_RegionSetupDefaults;
	
	[Attribute("", UIWidgets.Object, "List of region objects defining settings for each region")]
	ref array<ref SCR_BuildingRegionSetupMulti> m_RegionSetupOverride;
	
	[Attribute("", UIWidgets.Object, "List of region interconnections")]
	ref array<ref SCR_BuildingLinkRegion> m_RegionLinks;
	
	ResourceName m_sConfigPath;
	
	vector m_vBuildingMins;
	vector m_vBuildingMaxs;
	
	ref array<ref SCR_BuildingRegion> m_Regions = new array<ref SCR_BuildingRegion>;
	
	[Attribute("", UIWidgets.None, "GENERATED ATTRIBUTE, HIDDEN FROM EDITOR")]
	ref array<ResourceName> m_DamagedRegionModels;
	[Attribute("", UIWidgets.None, "GENERATED ATTRIBUTE, HIDDEN FROM EDITOR")]
	ref array<ref SCR_BuildingResourceList> m_DamagedRegionIntersectDebrisModels;
	
	//------------------------------------------------------------------------------------------------
	void SetConfigPath(ResourceName path)
	{
		m_sConfigPath = path;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRegionNum()
	{
		if (!m_Regions)
			return 0;
		
		return m_Regions.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BuildingRegion GetRegionStruct(int regionNumber)
	{
		if (regionNumber >= m_Regions.Count())
			return null;
		else
			return m_Regions.Get(regionNumber);
	}
	
	//------------------------------------------------------------------------------------------------
	void FillDamagedModels()
	{
		Print("SCR_BuildingSetup::FillDamagedModels: Filling damaged model lists...");
		
		if (!m_DamagedRegionModels)
			m_DamagedRegionModels = new array<ResourceName>;
		if (!m_DamagedRegionIntersectDebrisModels)
			m_DamagedRegionIntersectDebrisModels = new array<ref SCR_BuildingResourceList>;
		m_DamagedRegionModels.Clear();
		m_DamagedRegionIntersectDebrisModels.Clear();
		
		int numRegions = GetRegionNum();
		for (int region = 0; region < numRegions; region++)
		{
			// Add damaged region model path
			ResourceName resPath = ResourceName.Empty;
			Resource resource;
			#ifdef WORKBENCH
				if (m_ModelDamagedPartBase != string.Empty)
				{
					if (region < 10)
						resPath = Workbench.GetResourceName(m_ModelDamagedPartBase + "0" + region.ToString() + ".xob");
					else
						resPath = Workbench.GetResourceName(m_ModelDamagedPartBase + region.ToString() + ".xob");
					resource = Resource.Load(resPath);
					if (!resource.IsValid())
						resPath = ResourceName.Empty;
				}
			#endif
			m_DamagedRegionModels.Insert(resPath);
			
			SCR_BuildingResourceList resList = new SCR_BuildingResourceList;
			resList.m_aResources = new array<ResourceName>;
			
			// Add damaged region intersection debris model path
			for (int otherRegion = 0; otherRegion < numRegions; otherRegion++)
			{
				resPath = ResourceName.Empty;
				#ifdef WORKBENCH
					if (m_ModelRegionIntersectDebrisBase != string.Empty)
					{
						if (region != otherRegion)
						{
							if (region < 10)
							{
								if (otherRegion < 10)
									resPath = Workbench.GetResourceName(m_ModelRegionIntersectDebrisBase + "0" + region.ToString() + m_ModelRegionIntersectDebrisPrefix + "0" + otherRegion.ToString() + ".xob");
								else
									resPath = Workbench.GetResourceName(m_ModelRegionIntersectDebrisBase + "0" + region.ToString() + m_ModelRegionIntersectDebrisPrefix + otherRegion.ToString() + ".xob");
							}
							else
							{
								if (otherRegion < 10)
									resPath = Workbench.GetResourceName(m_ModelRegionIntersectDebrisBase + region.ToString() + m_ModelRegionIntersectDebrisPrefix + "0" + otherRegion.ToString() + ".xob");
								else
									resPath = Workbench.GetResourceName(m_ModelRegionIntersectDebrisBase + region.ToString() + m_ModelRegionIntersectDebrisPrefix + otherRegion.ToString() + ".xob");
							}
						}
						resource = Resource.Load(resPath);
						if (!resource.IsValid())
							resPath = ResourceName.Empty;
					}
				#endif
				resList.m_aResources.Insert(resPath);
			}
			m_DamagedRegionIntersectDebrisModels.Insert(resList);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDamagedRegionModel(int region)
	{
		if (m_DamagedRegionModels.IsEmpty())
		{
			Print("SCR_BuildingSetup::GetDamagedRegionModel: Models not stored for config '" + m_sConfigPath + "', building will not function correctly in built game!", LogLevel.WARNING);
			#ifdef WORKBENCH
				FillDamagedModels();
				if (m_DamagedRegionModels.IsEmpty())
					return ResourceName.Empty;
			#else
				return ResourceName.Empty;
			#endif
		}
		
		return m_DamagedRegionModels[region];
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRegionIntersectDebrisModel(int region, int otherRegion)
	{
		if (m_DamagedRegionIntersectDebrisModels.IsEmpty())
		{
			Print("SCR_BuildingSetup::GetRegionIntersectDebrisModel: Models not stored for config '" + m_sConfigPath + "', building will not function correctly in built game!", LogLevel.WARNING);
			#ifdef WORKBENCH
				FillDamagedModels();
				if (m_DamagedRegionIntersectDebrisModels.IsEmpty())
					return ResourceName.Empty;
			#else
				return ResourceName.Empty;
			#endif
		}
		
		SCR_BuildingResourceList resList = m_DamagedRegionIntersectDebrisModels[region];
		return resList.m_aResources[otherRegion];
	}
	
	//------------------------------------------------------------------------------------------------
	private void ConnectStructuralRegions(int parentIndex, int childIndex)
	{
		SCR_BuildingRegion regionParent = m_Regions.Get(parentIndex);
		SCR_BuildingRegion regionChild = m_Regions.Get(childIndex);
		
		if (!regionParent || !regionChild)
			return;
		
		regionParent.AddConnectionOut(regionChild);
		regionChild.AddConnectionIn(regionParent);
	}
	
	//------------------------------------------------------------------------------------------------
	void Cleanup()
	{
		while (m_Regions.Count() > 0)
		{
			SCR_BuildingRegion region = m_Regions.Get(0);
			m_Regions.Remove(0);
			delete region;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBoundingRegionNumber(vector localPos)
	{
		int numRegions = GetRegionNum();
		for (int i = 0; i < numRegions; i++)
		{
			SCR_BuildingRegion regionStruct = GetRegionStruct(i);
			vector mins = regionStruct.GetRegionPos() + regionStruct.GetRegionSize() * -0.5;
			vector maxs = regionStruct.GetRegionPos() + regionStruct.GetRegionSize() * 0.5;
			if (SCR_Global.IntersectBoxPoint(localPos, mins, maxs))
				return regionStruct.GetRegionIndex();
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	void CalculateRegionCentersAndSize(GenericEntity buildingMdl)
	{
		Resource resource = Resource.Load(m_ModelUndamaged);
		VObject asset;
		
		if (resource.IsValid())
		{
			asset = resource.GetResource().ToVObject();
			buildingMdl.SetObject(asset, "");
			buildingMdl.GetBounds(m_vBuildingMins, m_vBuildingMaxs);
		}
		
		int numRegions = GetRegionNum();
		for (int i = 0; i < numRegions; i++)
		{
		
			resource = Resource.Load(GetDamagedRegionModel(i));
			if (resource.GetResource())
			{
				asset = resource.GetResource().ToVObject();
				if (asset)
				{				
					buildingMdl.SetObject(asset, "");
					
					vector mins, maxs;
					buildingMdl.GetBounds(mins, maxs);
					
					// Store region size and pos
					SCR_BuildingRegion regionStruct = GetRegionStruct(i);
					
					regionStruct.SetRegionPos((maxs - mins) * 0.5 + mins);
					regionStruct.SetRegionSize(maxs - mins);
					
					continue;
				}
			}
			
			Print("BUILDING_SETUP::CalculateRegionCentersAndSize(): Failed to load model path " + GetDamagedRegionModel(i));
			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets whether debris effects such as particle effects etc are to be spawned on destruction
	void SetNoDebrisFX(bool noDebrisFX)
	{
		m_bNoDebrisFX = noDebrisFX;
	}
	//------------------------------------------------------------------------------------------------
	//! Gets the matrix for spawning debris
	void GetDebrisRegionMatrix(IEntity buildingEnt, int regionNumber, out vector fxMat[4])
	{
		SCR_BuildingRegion regionStruct = GetRegionStruct(regionNumber);
		buildingEnt.GetWorldTransform(fxMat);
		fxMat[3] = buildingEnt.CoordToParent(regionStruct.GetRegionPos());
	}
	
	//------------------------------------------------------------------------------------------------
	// Spawns debris objects for each region
	void SpawnDebrisForRegion(IEntity buildingEnt, int region, vector baseLinVel, vector baseAngVel)
	{
		if (m_bNoDebrisFX)
			return;
		
		SCR_BuildingRegion regionStruct = GetRegionStruct(region);
		if (!regionStruct)
			return;
		
		vector fxMat[4];
		GetDebrisRegionMatrix(buildingEnt, region, fxMat);
		
		ResourceName ptcPath;
		ResourceName sndPath;
		ResourceName pfbPath;
		
		// Spawn destruction effects, if defined
		if (!regionStruct.GetDestructFX(ptcPath, sndPath, pfbPath))
			return;
		
		if (ptcPath != "")
		{
			ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
			spawnParams.Transform[3] = fxMat[3];
			spawnParams.UseFrameEvent = true;
			ParticleEffectEntity.SpawnParticleEffect(ptcPath, spawnParams);
		}
		
		// TODO: Implement playing sound
		
		if (pfbPath != "")
		{
			Resource resource = Resource.Load(pfbPath);
			if (resource.IsValid())
			{
				IEntity pfbParent = GetGame().SpawnEntityPrefab(resource);
				SCR_EntityHelper.SetHierarchyTransform(pfbParent, fxMat);
			}
		}
		
		const int num = 10;
		const float radius = 1.5;
		
		for (int i = 0; i < num; i++)
		{
			vector debrisMat[4];
			Math3D.AnglesToMatrix(Vector(Math.RandomFloat(-180, 180), Math.RandomFloat(-90, 90), Math.RandomFloat(-180, 180)), debrisMat);
			debrisMat[3] = debrisMat[0] * Math.RandomFloat(-radius, radius) + debrisMat[1] * Math.RandomFloat(-radius, radius) + debrisMat[2] * Math.RandomFloat(-radius, radius) + fxMat[3];
			
			float rndScale = Math.RandomFloat(0.8, 1.5) * 0.5;
			debrisMat[0] = debrisMat[0] * rndScale;
			debrisMat[1] = debrisMat[1] * rndScale;
			debrisMat[2] = debrisMat[2] * rndScale;
			
			ResourceName debrisPath;
			float debrisMass;
			float debrisLifetime;
			float debrisMaxDist;
			float debrisSpeed;
			int debrisPriority;
			
			if (Math.RandomFloat01() < 0.4) // Wood bits
			{
				if (Math.RandomFloat01() < 0.1) // Fence part
				{
					debrisPath = "{1121FA3793FC3796}Assets/Tests/A2OA/Debris/debris_K_03.xob";
					debrisMass = 5;
					debrisLifetime = Math.RandomFloat(5, 10);
					debrisMaxDist = 150;
					debrisSpeed = Math.RandomFloat(2, 10);
					debrisPriority = 2;
				}
				else if (Math.RandomFloat01() < 0.25) // Pylon part
				{
					debrisPath = "{16E60066F57A8D55}Assets/Tests/A2OA/Debris/debris_K_04.xob";
					debrisMass = 25;
					debrisLifetime = Math.RandomFloat(8, 12);
					debrisMaxDist = 200;
					debrisSpeed = Math.RandomFloat(2, 7);
					debrisPriority = 2;
				}
				else if (Math.RandomFloat01() < 0.4) // Log part
				{
					debrisPath = "{61EC68EE01D86225}Assets/Tests/A2OA/Debris/debris_K_05.xob";
					debrisMass = 5;
					debrisLifetime = Math.RandomFloat(5, 10);
					debrisMaxDist = 150;
					debrisSpeed = Math.RandomFloat(5, 15);
					debrisPriority = 1;
				}
				else if (Math.RandomFloat01() < 0.4) // Small plank
				{
					debrisPath = "{F8F2D1771C3F53B5}Assets/Tests/A2OA/Debris/debris_K_06.xob";
					debrisMass = 5;
					debrisLifetime = Math.RandomFloat(5, 10);
					debrisMaxDist = 150;
					debrisSpeed = Math.RandomFloat(5, 15);
					debrisPriority = 1;
				}
				else // Large plank
				{
					debrisPath = "{F77D25D5D1322633}Assets/Tests/A2OA/Debris/debris_K_08.xob";
					debrisMass = 10;
					debrisLifetime = Math.RandomFloat(10, 14);
					debrisMaxDist = 200;
					debrisSpeed = Math.RandomFloat(3, 8);
					debrisPriority = 3;
				}
			}
			else // Brick blocks
			{
				if (Math.RandomFloat01() < 0.4) // Small block
				{
					debrisPath = "{FF352B267AB9E976}Assets/Tests/A2OA/Debris/debris_K_01.xob";
					debrisMass = 10;
					debrisLifetime = Math.RandomFloat(5, 10);
					debrisMaxDist = 100;
					debrisSpeed = Math.RandomFloat(2, 8);
					debrisPriority = 1;
				}
				else if (Math.RandomFloat01() < 0.5) // Large block 1
				{
					debrisPath = "{662B92BF675ED8E6}Assets/Tests/A2OA/Debris/debris_K_02.xob";
					debrisMass = 50;
					debrisLifetime = Math.RandomFloat(8, 12);
					debrisMaxDist = 200;
					debrisSpeed = Math.RandomFloat(1, 5);
					debrisPriority = 3;
				}
				else // Large block 2
				{
					debrisPath = "{8FF8B9FFE89DBCC5}Assets/Tests/A2OA/Debris/debris_K_07.xob";
					debrisMass = 100;
					debrisLifetime = Math.RandomFloat(10, 14);
					debrisMaxDist = 250;
					debrisSpeed = Math.RandomFloat(1, 5);
					debrisPriority = 3;
				}
			}
			
			vector rndLinVel = debrisMat[3] - buildingEnt.GetOrigin();
			rndLinVel = rndLinVel.Normalized() * debrisSpeed + baseLinVel;
			vector rndAngVel = Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * Math.RandomFloat(90, 720) + baseAngVel;
#ifdef ENABLE_BASE_DESTRUCTION
			SCR_DebrisSmallEntity.SpawnDebris(buildingEnt.GetWorld(), debrisMat, debrisPath, debrisMass, debrisLifetime, debrisMaxDist, debrisPriority, rndLinVel, rndAngVel);
#endif
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		// Create our regions
		for (int i = 0; i < m_iNumRegions; i++)
		{
			SCR_BuildingRegion regionStruct = new SCR_BuildingRegion;
			regionStruct.SetRegionIndex(i);
			m_Regions.Insert(regionStruct);
			
			// Set defaults
			if (m_RegionSetupDefaults)
			{
				regionStruct.SetMaxHealth(m_RegionSetupDefaults.m_MaxHealth);
				regionStruct.SetStructuralSupportPercentage(m_RegionSetupDefaults.m_StructuralSupportPct);
				regionStruct.SetDestructFX(m_RegionSetupDefaults.m_DestructFX_PTC, m_RegionSetupDefaults.m_DestructFX_SND, m_RegionSetupDefaults.m_DestructFX_PFB);
			}
		}
		
		// Set settings for regions
		int cnt = m_RegionSetupOverride.Count();
		for (int i = 0; i < cnt; i++)
		{
			SCR_BuildingRegionSetupMulti setupMulti = m_RegionSetupOverride.Get(i);
			int forRegionNum = setupMulti.m_aIndexes.Count();
			for (int n = 0; n < forRegionNum; n++)
			{
				int rIndex = setupMulti.m_aIndexes.Get(n);
				if (rIndex < 0 || rIndex >= m_iNumRegions)
					continue;
				
				SCR_BuildingRegion regionStruct = m_Regions.Get(rIndex);
				if (regionStruct)
				{
					regionStruct.SetMaxHealth(setupMulti.m_MaxHealth);
					regionStruct.SetStructuralSupportPercentage(setupMulti.m_StructuralSupportPct);
					regionStruct.SetDestructFX(setupMulti.m_DestructFX_PTC, setupMulti.m_DestructFX_SND, setupMulti.m_DestructFX_PFB);
				}
			}
		}
		
		// Create a temporary entity that uses the damaged model, to get the bone positions etc
		GenericEntity buildingMdl = GenericEntity.Cast(GetGame().SpawnEntity(GenericEntity));
		
		CalculateRegionCentersAndSize(buildingMdl);
		
		// Delete the temporary entity here
		if (buildingMdl)
		{
			delete buildingMdl;
			buildingMdl = null;
		}
		
		cnt = m_Regions.Count();
		for (int i = 0; i < cnt; i++)
		{
			SCR_BuildingRegion rgn = m_Regions.Get(i);
			rgn.SetRegionIndex(i);
		}
		
		cnt = m_RegionLinks.Count();
		for (int i = 0; i < cnt; i++)
		{
			SCR_BuildingLinkRegion link = m_RegionLinks.Get(i);
			int chldCnt = link.m_aChildren.Count();
			for (int n = 0; n < chldCnt; n++)
			{
				ConnectStructuralRegions(link.m_iParent, link.m_aChildren.Get(n));
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BuildingSetup()
	{
		Cleanup();
	}
};
