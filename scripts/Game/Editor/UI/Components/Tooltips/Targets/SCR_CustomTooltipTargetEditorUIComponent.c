//! @ingroup Editor_UI Editor_UI_Components

class SCR_CustomTooltipTargetEditorUIComponent : SCR_BaseTooltipTargetEditorUIComponent
{
	[Attribute()]
	protected ref SCR_UIInfo m_Info;
	
	//------------------------------------------------------------------------------------------------
	override SCR_UIInfo GetInfo()
	{
		return m_Info;
	}
}
