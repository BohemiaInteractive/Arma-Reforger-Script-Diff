//! Returns true if VoN UI is active
[BaseContainerProps(), BaseContainerCustomStringTitleField("Radial menu Active")]
class SCR_RadialMenuActiveActionCondition: SCR_AvailableActionCondition
{
	protected SCR_RadialMenu m_RadialMenu;
	
	//~Todo: Check if command menu is active
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		if (!m_RadialMenu)
		{
			m_RadialMenu = SCR_RadialMenu.GlobalRadialMenu();
			
			if (!m_RadialMenu)
				return GetReturnResult(false);
		}
		
		return GetReturnResult(m_RadialMenu.IsOpened());
	}
};