[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_ItemHintUIInfo : SCR_InventoryUIInfo
{
	protected ResourceName m_Layout = "{9996B50BE8DFED5E}UI/layouts/Menus/Inventory/InventoryItemHintLayout.layout";
	
	[Attribute(desc: "Imageset with icon for the itemHint", params: "imageset")]
	protected ResourceName m_sImageSet;
	
	[Attribute("", desc: "", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sHintText;
		
	[Attribute("", desc: "", uiwidget: UIWidgets.EditBox)]
	protected string m_sImageTileName;
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetHintResource()
	{
		return m_Layout;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetImageSet()
	{
		return m_sImageSet;
	}

	//------------------------------------------------------------------------------------------------
	string GetImageName()
	{
		return m_sImageTileName;
	}

	//------------------------------------------------------------------------------------------------
	LocalizedString GetHintText()
	{
		return m_sHintText;
	}
}
