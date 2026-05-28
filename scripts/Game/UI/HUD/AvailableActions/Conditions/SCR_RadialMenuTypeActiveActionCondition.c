//! Returns true if VoN UI is active
[BaseContainerProps(), BaseContainerCustomStringTitleField("Radial menu of type active")]
class SCR_RadialMenuTypeActiveActionCondition: SCR_AvailableActionCondition
{
	protected SCR_RadialMenu m_RadialMenu;
	
	[Attribute(defvalue: SCR_ERadialMenuType.DEFAULT.ToString(), desc: "Radial Menu Type", uiwidget: UIWidgets.ComboBox, enumType: SCR_ERadialMenuType)]
	protected SCR_ERadialMenuType m_eRadialMenuType;
	
	//~Todo: Check if command menu is active
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		if (!m_RadialMenu)
		{
			m_RadialMenu = SCR_RadialMenu.GetOpenedRadialMenu();
			
			if (!m_RadialMenu)
				return GetReturnResult(false);
		}
		
		return GetReturnResult(m_RadialMenu.m_eRadialType == m_eRadialMenuType && m_RadialMenu.IsOpened());
	}
}