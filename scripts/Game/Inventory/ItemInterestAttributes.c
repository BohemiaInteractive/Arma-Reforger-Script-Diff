class SCR_ItemOfInterestAttribute: BaseItemAttributeData
{
	[Attribute("999", UIWidgets.ComboBox, "Slot size", "", ParamEnumArray.FromEnum(ECommonItemType))]
	ECommonItemType m_InterestType;	
	
	ECommonItemType GetInterestType() 
	{
		return m_InterestType; 
	}
};
