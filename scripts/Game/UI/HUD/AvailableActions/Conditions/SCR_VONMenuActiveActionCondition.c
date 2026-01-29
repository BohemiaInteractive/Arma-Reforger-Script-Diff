//! Returns true if VON Menu is active
[BaseContainerProps(), BaseContainerCustomStringTitleField("VON menu active")]
class SCR_VONMenuActiveActionCondition: SCR_AvailableActionCondition
{
	protected SCR_RadialMenu m_RadialMenu;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		if (!m_RadialMenu)
		{
			SCR_VONController vonController = SCR_VONController.Cast(GetGame().GetPlayerController().FindComponent(SCR_VONController));
			if (vonController && vonController.GetVONMenu())
				m_RadialMenu = vonController.GetVONMenu().GetRadialMenu();
			
			if (!m_RadialMenu)
				return GetReturnResult(false);
		}
		
		return GetReturnResult(m_RadialMenu.IsOpened());
	}
};