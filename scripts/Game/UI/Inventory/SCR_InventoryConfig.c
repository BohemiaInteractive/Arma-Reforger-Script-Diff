[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sName", true)]
class SCR_LoadoutArea
{
	[Attribute("Name", desc: "")]
	protected string m_sName;
	
	[Attribute( defvalue: "", uiwidget: UIWidgets.Object, desc: "Area type" )]
	ref LoadoutAreaType m_LoadoutArea;
	
	[Attribute( defvalue: "0", uiwidget: UIWidgets.SearchComboBox, desc: "Area", enums: ParamEnumArray.FromEnum( ECommonItemType ) )]
	ECommonItemType m_eCommonType;
	
	[Attribute( defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Row in inventory list" )]
	private int m_iRow;
	
	[Attribute( defvalue: "", desc: "Icon for the Area", params: "edds" )]
	ResourceName m_sIcon;
	
	//------------------------------------------------------------------------
	//! If false will hide the slot. Note that the SCR_EquipmentStorageComponent will still need override CanStoreItem CanStoreResource to disable the slot fully
	//! \return True if the slot is valid and can be displayed
	bool IsValid()
	{
		return true;
	}	
};

[BaseContainerProps(configRoot: true)]
class SCR_InventoryConfig
{
	[Attribute( desc: "Loadout Areas" )]
	protected ref array<ref SCR_LoadoutArea> m_aLoadoutAreas;
	
	//------------------------------------------------------------------------
	int GetValidLoadoutAreaCount()
	{
		array<SCR_LoadoutArea> validLoadoutAreas = {};
		return GetValidLoaddoutAreas(validLoadoutAreas);
	}
	
	//------------------------------------------------------------------------
	int GetValidLoaddoutAreas(notnull inout array<SCR_LoadoutArea> validLoadoutAreas)
	{
		validLoadoutAreas.Clear();
		
		foreach(SCR_LoadoutArea areaEntry : m_aLoadoutAreas)
		{
			if (!areaEntry.IsValid())
				continue;
			
			validLoadoutAreas.Insert(areaEntry);
		}
		
		return validLoadoutAreas.Count();
	}
	
	//------------------------------------------------------------------------
	int GetRowByArea( LoadoutAreaType pArea )
	{ 
		if (!pArea)
			return -1;
		
		array<SCR_LoadoutArea> validLoadoutAreas = {};
		GetValidLoaddoutAreas(validLoadoutAreas);
		
		foreach(int iIndex, SCR_LoadoutArea pAreaEntry : validLoadoutAreas)
		{
			if (pAreaEntry.m_LoadoutArea && pAreaEntry.m_LoadoutArea.IsInherited(pArea.Type()))
				return iIndex; 
		}
			
		return -1;
	}
	
	//------------------------------------------------------------------------
	int GetRowByCommonItemType( ECommonItemType pItemType )
	{ 
		array<SCR_LoadoutArea> validLoadoutAreas = {};
		GetValidLoaddoutAreas(validLoadoutAreas);
		
		foreach(int iIndex, SCR_LoadoutArea pAreaEntry : validLoadoutAreas)
		{
			if (pAreaEntry.m_eCommonType == pItemType)
				return iIndex; 
		}
			
		return -1;
	}
	
	//------------------------------------------------------------------------
	ResourceName GetIcon( LoadoutAreaType pArea )
	{
		if (!pArea)
			return ResourceName.Empty;
		
		array<SCR_LoadoutArea> validLoadoutAreas = {};
		GetValidLoaddoutAreas(validLoadoutAreas);
		
		foreach (int iIndex, SCR_LoadoutArea pAreaEntry : validLoadoutAreas)
		{
			if (pAreaEntry.m_LoadoutArea && pAreaEntry.m_LoadoutArea.IsInherited(pArea.Type()))
				return pAreaEntry.m_sIcon;
		}
		
		return ResourceName.Empty;
	}
	
	//------------------------------------------------------------------------
	ResourceName GetIconByRow( int iIndex )
	{
		array<SCR_LoadoutArea> validLoadoutAreas = {};
		GetValidLoaddoutAreas(validLoadoutAreas);
		
		if (iIndex > validLoadoutAreas.Count() -1)
			return ResourceName.Empty;
		
		return validLoadoutAreas[ iIndex ].m_sIcon;
	}
	
	//------------------------------------------------------------------------
	LoadoutAreaType GetAreaByRow( int iIndex )
	{
		array<SCR_LoadoutArea> validLoadoutAreas = {};
		GetValidLoaddoutAreas(validLoadoutAreas);
		
		if (!validLoadoutAreas.IsIndexValid(iIndex))
			return null;
		
		return validLoadoutAreas[iIndex].m_LoadoutArea;
	}
	
	//------------------------------------------------------------------------
	void ~SCR_InventoryConfig()
	{
		m_aLoadoutAreas = null;
	}
};

