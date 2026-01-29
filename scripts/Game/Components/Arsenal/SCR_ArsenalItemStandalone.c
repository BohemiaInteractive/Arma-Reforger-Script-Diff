//! Arsenal Item to be added in the configs outside of The faction Catalog
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_ItemResourceName", true)]
class SCR_ArsenalItemStandalone : SCR_ArsenalItem
{
	[Attribute(desc: "Prefab of the Arsenal item.", params: "et")]
	protected ResourceName m_ItemResourceName;
	
	//------------------------------------------------------------------------------------------------
	override ResourceName GetItemResourceName()
	{
		return m_ItemResourceName;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_ArsenalItemStandalone()
	{
		if (GetItemResourceName().IsEmpty())
			return;

		m_ItemResource = Resource.Load(GetItemResourceName());
	}
}
