/**
Multilist catalog for ease of use for developers. This will still merge all lists into the m_aEntityEntryList on init and set m_aMultiLists to null
*/
[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomEntityCatalogCatalog(EEntityCatalogType, "m_eEntityCatalogType", "m_aEntityEntryList", "m_aMultiLists")]
class SCR_EntityCatalogMultiList: SCR_EntityCatalog
{
	[Attribute(desc: "An array of all multi lists which in turn hold lists of entries. Used for readablility for devs and each works the same as the m_aEntityEntryList. You are free to make, delete, rename and move lists as you want and names do not need to be unique. Note that this array will be null after Init as it will be merged into the m_aEntityEntryList")]
	protected ref array <ref SCR_EntityCatalogMultiListEntry> m_aMultiLists;
	
	//------------------------------------------------------------------------------------------------
	//! Only valid before init. Used when merging entity lists
	//! param[in] multiLists Return list of all the multilists
	//! return Count of multiLists
	int GetMultiList(notnull out array <SCR_EntityCatalogMultiListEntry> multiLists)
	{
		multiLists.Clear();
		
		if (!m_aMultiLists)
			return 0;
		
		foreach (SCR_EntityCatalogMultiListEntry entry : m_aMultiLists)
		{
			multiLists.Insert(entry);
		}
		
		return multiLists.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	override void ClearCatalogOnMerge()
	{
		super.ClearCatalogOnMerge();
		
		m_aMultiLists.Clear();
	}
	
	//======================================== INIT ========================================\\
	protected override void InitCatalog()
	{
		//~ Merge entries lists into the m_aEntityEntryList so the system can use it
		foreach (SCR_EntityCatalogMultiListEntry multiList: m_aMultiLists)
		{
			foreach (SCR_EntityCatalogEntry entry : multiList.m_aEntities)
			{
				//~ Ignore disabled entries
				if (!entry.IsEnabled())
					continue;
				
				m_aEntityEntryList.Insert(entry);
			}
		}
		
		//~ Clear Multilist array as no need to keep data
		m_aMultiLists = null;
		
		//~ Default init
		super.InitCatalog();
	}
}

//--------------------------------- Multi List entry  ---------------------------------\\
[BaseContainerProps(), SCR_BaseContainerCustomEntityCatalogMultiListEntry("m_sIdentifier", "m_aEntities")]
class SCR_EntityCatalogMultiListEntry
{
	[Attribute("UNKNOWN", desc: "For devs only to understand which entries are in the list")]
	protected string m_sIdentifier;
	
	[Attribute(desc: "List of entries. This list will be merged into the m_aEntityEntryList on init. You can still add entries to the m_aEntityEntryList as those will NOT be cleared")]
	ref array<ref SCR_EntityCatalogEntry> m_aEntities;
}

//--------------------------------- Custom Attribute for MultiList ---------------------------------\\
class SCR_BaseContainerCustomEntityCatalogMultiListEntry : BaseContainerCustomTitle
{
	protected string m_sPropertyName;
	protected string m_sEntityListName;

	void SCR_BaseContainerCustomEntityCatalogMultiListEntry(string propertyName, string entityListName)
	{
		m_sPropertyName = propertyName;
		m_sEntityListName = entityListName;
	}

	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		bool hasError = false;
		string propertyName;
		if (!source.Get(m_sPropertyName, propertyName))
		{
			return false;
		}
		
		//~ Get entry count
		array<ref SCR_EntityCatalogEntry> entityList;
		if (!source.Get(m_sEntityListName, entityList))
			return false; 
		
		int enabledCount = 0;
		
		//~ Get count of enabled entries
		foreach (SCR_EntityCatalogEntry entity: entityList)
		{
			if (entity.IsEnabled())
			{
				enabledCount++;
				
				//~ Is Empty show error warning in title
				if (entity.GetPrefab() == "")
					hasError = true;
			}
		}
		
		string format;
		//~ Does not have disabled entries
		if (enabledCount == entityList.Count())
			format = "%1 (%2)";
		//~ Has disabled entries
		else 
			format = "%1 (%3 of %2)";
		
		if (hasError)
			format += " !!";

		title = string.Format(format, propertyName, entityList.Count(), enabledCount);
		return true;
	}
};