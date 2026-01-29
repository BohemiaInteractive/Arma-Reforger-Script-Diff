//------------------------------------------------------------------------------------------------
class SCR_BuildingConfigManagerEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingConfigManagerEntity : GenericEntity
{
	static SCR_BuildingConfigManagerEntity s_Instance = null;
	
	ref map<ResourceName, ref SCR_BuildingSetup> m_BuildingSetups = new map<ResourceName, ref SCR_BuildingSetup>();
	
	//------------------------------------------------------------------------------------------------
	static void CreateInstance()
	{
		if (!s_Instance)
			GetGame().SpawnEntity(SCR_BuildingConfigManagerEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_BuildingSetup ReloadConfig(ResourceName configPath)
	{
		if (configPath == string.Empty)
			return null;
		
		if (!s_Instance)
			return null;
		
		s_Instance.m_BuildingSetups.Remove(configPath);
		
		return GetBuildingSetupFromConfig(configPath);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_BuildingSetup GetBuildingSetupFromConfig(ResourceName configPath)
	{
		if (configPath == string.Empty)
			return null;
		
		CreateInstance();
		
		SCR_BuildingSetup buildSetup = s_Instance.m_BuildingSetups.Get(configPath);
		if (!buildSetup)
		{
			Resource holder = BaseContainerTools.LoadContainer(configPath);
			if (!holder)
				return null;
			
			buildSetup = SCR_BuildingSetup.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			if (!buildSetup)
				return null;
			
			buildSetup.SetConfigPath(configPath);
			buildSetup.Init();
			s_Instance.m_BuildingSetups.Insert(configPath, buildSetup);
		}
		
		return buildSetup;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BuildingConfigManagerEntity(IEntitySource src, IEntity parent)
	{
		s_Instance = this;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BuildingConfigManagerEntity()
	{
		if (s_Instance == this)
			s_Instance = null;
	}
};