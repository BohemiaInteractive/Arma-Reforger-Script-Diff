class SCR_AlternativeModel: BaseItemAttributeData
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Alternative model to be filled", params: "xob")]
	private ResourceName m_sAlternativeModel;
	
	//------------------------------------------------------------------------------------------------	
	ResourceName GetAlternativeModel() 
	{
		return m_sAlternativeModel;
	}
};
