[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EArsenalItemDisplayType, "m_eArsenalItemDisplayType")]
class SCR_ArsenalItemDisplayData
{	
	[Attribute("0", desc: "Type of display that the SCR_ArsenalDisplayComponent allows", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EArsenalItemDisplayType),  category: "Display settings")]
	protected EArsenalItemDisplayType m_eArsenalItemDisplayType;
	
	[Attribute("0 0 0", desc: "Offset of item when displaye", uiwidget: UIWidgets.EditBox, category: "Display settings")]
	protected vector m_vItemOffset;
	
	[Attribute("0 0 0", desc: "Rotation of item when displayed", uiwidget: UIWidgets.EditBox, category: "Display settings")]
	protected vector m_vItemRotation;

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetItemOffset()
	{
		return m_vItemOffset;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetItemRotation()
	{
		return m_vItemRotation;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EArsenalItemDisplayType GetDisplayType()
	{
		return m_eArsenalItemDisplayType;
	}
}

enum EArsenalItemDisplayType
{
	WEAPON_RACK,
}
