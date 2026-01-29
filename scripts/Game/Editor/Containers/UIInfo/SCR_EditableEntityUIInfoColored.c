[BaseContainerProps()]
class SCR_EditableEntityUIInfoColored : SCR_EditableEntityUIInfo
{
	[Attribute("1 1 1 1", desc: "Will overwrite the color in the content browser to display custom colors. Created for System entities")]
	protected ref Color m_ContentBrowserColorOverwrite;
	
	//------------------------------------------------------------------------------------------------
	//! \return Overwrite Color
	Color GetOverwriteColor()
	{
		return Color.FromInt(m_ContentBrowserColorOverwrite.PackToInt());
	}
}
