[BaseContainerProps()]
class SCR_EditorModeUIInfo : SCR_UIInfo
{
	[Attribute()]
	protected ref Color m_ModeColor;

	[Attribute()]
	protected int m_iOrder;

	//------------------------------------------------------------------------------------------------
	Color GetModeColor()
	{
		return Color.FromInt(m_ModeColor.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	int GetOrder()
	{
		return m_iOrder;
	}
}
