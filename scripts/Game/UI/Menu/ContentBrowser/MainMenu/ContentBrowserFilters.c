/*
Classes for filters of workshop.
*/


//--------------------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_FilterEntryTitle()]
class SCR_ContentBrowserFilterTag : SCR_FilterEntry
{
	// Tag registered in Workshop API
	WorkshopTag m_Tag;
	
	// Default thumbnail associated with this tag
	[Attribute("", UIWidgets.ResourceNamePicker, "Default thumbnail associated with this tag", params: "edds")]
	ResourceName m_sDefaultThumbnail;
	
	//--------------------------------------------------------------------------------------------------------------
	WorkshopTag GetWorkshopTag()
	{
		return m_Tag;
	}
	
	
	//--------------------------------------------------------------------------------------------------------------
	void RegisterInWorkshopApi(WorkshopApi api)
	{
		if (!m_Tag)
			m_Tag = api.RegisterTag(m_sInternalName);
	}
};