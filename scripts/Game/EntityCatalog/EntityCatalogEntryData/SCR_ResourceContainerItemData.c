[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Resource Container Data", "DISABLED - Resource Container Data", 1)]
class SCR_ResourceContainerItemData : SCR_BaseEntityCatalogData
{
	[Attribute(defvalue: EResourceType.SUPPLIES.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sets the type of Resource to be used.\nOnly a transaction matching Resource types can be successfully concluded.", enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceType;
	
	//~ Max Resource Value set by InitData()
	protected int m_iMaxResourceValue = -1;
	
	//--------------------------------- Get resource amount ---------------------------------\\
	/*!
	Get max resource value of container
	Data obtained from prefab, -1 if no max resource value found
	\return max Resource value
	*/
	int GetMaxResourceValue()
	{		
		return m_iMaxResourceValue;
	}
	
	//--------------------------------- Set resource amount ---------------------------------\\
	//~ Set the resource amount. Done on InitData() taking the info from the ResourceComponent
	protected void SetMaxResourceValue(int maxResourceValue)
	{
		m_iMaxResourceValue = maxResourceValue;
	}
	
	//--------------------------------- Get Resource Container ---------------------------------\\
	//~ Get container data from SCR_ResourceComponent on Prefab
	//~ Called on InitData()
	protected SCR_ResourceContainer GetResourceContainer(notnull SCR_EntityCatalogEntry entry, bool showError = true)
	{		
		//~ Load Prefab
		Resource entityPrefab = Resource.Load(entry.GetPrefab());
		if (!entityPrefab)
		{
			if (showError)
				Debug.Error2("SCR_ResourceContainerItemData", "Data '" + entry.GetPrefab() + "' (index: " +  entry.GetCatalogIndex() + ") in catalog '" + typename.EnumToString(EEntityCatalogType, entry.GetCatalogParent().GetCatalogType()) + "' has an invalid prefab!");
			
			return null;
		}
			
		//~ Get entity source
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(entityPrefab);
		if (!entitySource)
		{
			if (showError)
				Debug.Error2("SCR_ResourceContainerItemData", "Data '" + entry.GetPrefab() + "' (index: " +  entry.GetCatalogIndex() + ") in catalog '" + typename.EnumToString(EEntityCatalogType, entry.GetCatalogParent().GetCatalogType()) + "' has an invalid prefab!");
			
			return null;
		}
		
		//~ Get SCR_ResourceComponent on prefab by looping through all components
		SCR_ResourceContainer resourceContainerTemp;
		SCR_ResourceContainer resourceContainer;
		int componentsCount = entitySource.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			//~ Check if correct component
			IEntityComponentSource componentSource = entitySource.GetComponent(i);
			if (componentSource.GetClassName().ToType().IsInherited(SCR_ResourceComponent))
			{
				//~ Grab the resource container BaseContainer
				BaseContainerList baseContainers = componentSource.GetObjectArray("m_aContainers");
				
				if (!baseContainers || baseContainers.Count() == 0)
				{
					if (showError)
						Debug.Error2("SCR_ResourceContainerItemData", "Data '" + entry.GetPrefab() + "' (index: " +  entry.GetCatalogIndex() + ") in catalog '" + typename.EnumToString(EEntityCatalogType, entry.GetCatalogParent().GetCatalogType()) + "' has 'SCR_ResourceComponent' but no SCR_ResourceContainer is assigned! (or container variable has been renamed)");
					
					return null;
				}
					
				for (int j = baseContainers.Count() - 1; j >= 0; j--)
				{
					resourceContainer = SCR_ResourceContainer.Cast(BaseContainerTools.CreateInstanceFromContainer(baseContainers.Get(j)));
					
					if (resourceContainer && resourceContainer.GetResourceType() == m_eResourceType)
						return resourceContainer;
					
				}
			}
		}
		
		if (showError)
			Debug.Error2("SCR_ResourceContainerItemData", "Data '" + entry.GetPrefab() + "' (index: " +  entry.GetCatalogIndex() + ") in catalog '" + typename.EnumToString(EEntityCatalogType, entry.GetCatalogParent().GetCatalogType()) + "' has no 'SCR_ResourceComponent'!");
		
		return null;
	}
	
	//--------------------------------- Init Data ---------------------------------\\
	//~ Set resource amount. Note: Ignored if data is disabled
	override void InitData(notnull SCR_EntityCatalogEntry entry)
	{
		//~ Safty: Already has Init called or invalid
		if (m_iMaxResourceValue != -1)
			return;
		
		//~ Get resource container and set max resource value
		SCR_ResourceContainer resourceContainer = GetResourceContainer(entry);
		if (resourceContainer)
			SetMaxResourceValue(resourceContainer.GetMaxResourceValue());
	}
};
