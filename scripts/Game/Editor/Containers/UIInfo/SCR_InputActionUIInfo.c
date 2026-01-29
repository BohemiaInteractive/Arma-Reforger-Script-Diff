[BaseContainerProps()]
class SCR_InputActionUIInfo : SCR_UIInfo
{
	[Attribute()]
	protected string m_sActionName;

	//------------------------------------------------------------------------------------------------
	string GetActionName()
	{
		return m_sActionName;
	}
}
