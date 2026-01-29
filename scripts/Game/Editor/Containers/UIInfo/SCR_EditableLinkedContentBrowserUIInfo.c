[BaseContainerProps()]
class SCR_EditableLinkedContentBrowserUIInfo : SCR_EditableEntityUIInfo
{
	[Attribute(desc: "The config which can be used for opening the content browser and only showing the correct labels and active filter.")]
	protected ref SCR_EditorContentBrowserDisplayConfig m_ContentBrowserLabelConfig;

	//------------------------------------------------------------------------------------------------
	//! Get the config which holds the content browser display data
	//! \return
	SCR_EditorContentBrowserDisplayConfig GetContentBrowserDisplayConfig()
	{
		return m_ContentBrowserLabelConfig;
	}
}
