[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EArsenalItemType, "m_ItemType")]
class SCR_ArsenalItemCountConfig
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_ItemType;

	[Attribute()]
	protected int m_MaxItemCount;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalItemType GetItemType()
	{
		return m_ItemType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMaxItemCount()
	{
		return m_MaxItemCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! Check if item type and current amount is valid for configured max count
	//! \param[in] maxItemCounts Source array with configured maximum count for specific SCR_EArsenalItemType
	//! \param[in] itemType SCR_EArsenalItemType of the item to check
	//! \param[in] currentCountTaken Current amount of times this item has been taken
	//! \return True when item does not have a max count configured, or is below max, false if array is null
	//!
	static bool CheckMaxCount(array<ref SCR_ArsenalItemCountConfig> maxItemCounts, SCR_EArsenalItemType itemType, int currentCountTaken)
	{
		if (!maxItemCounts)
			return false;
		
		for (int i = 0, c = maxItemCounts.Count(); i < c; i++)
		{
			SCR_ArsenalItemCountConfig item = maxItemCounts[i];
			if (item.GetItemType() == itemType
				&& currentCountTaken >= item.GetMaxItemCount())
			{
				return false;
			}
		}
		return true;
	}
}
