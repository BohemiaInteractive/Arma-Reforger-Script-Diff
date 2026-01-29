[BaseContainerProps()]
class SCR_InventoryUIInfo : UIInfo
{
	[Attribute("0", desc: "Set the icon visible in inventory hint.")]
	protected bool m_bShowIconInInventory;

	[Attribute("0 0 0 1")]
	protected ref Color m_IconColor;

	[Attribute(desc: "HintsArray")]
	protected ref array<ref SCR_InventoryItemHintUIInfo> m_aItemHints;
	
	//------------------------------------------------------------------------------------------------
	//! Function to override to get custom inventory name
	string GetInventoryItemName(InventoryItemComponent item)
	{
		return GetName();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Function to override to get custom inventory description
	string GetInventoryItemDescription(InventoryItemComponent item)
	{
		return GetDescription();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsIconVisible()
	{
		return m_bShowIconInInventory;
	}

	//------------------------------------------------------------------------------------------------
	Color GetIconColor()
	{
		return Color.FromInt(m_IconColor.PackToInt());
	}	

	//------------------------------------------------------------------------------------------------
	void GetItemHintArray(notnull out array<SCR_InventoryItemHintUIInfo> uiInfoArray)
	{
		uiInfoArray.Clear();
		
		foreach(SCR_InventoryItemHintUIInfo itemHint : m_aItemHints)
		{
			uiInfoArray.Insert(itemHint);
		}
	}
}