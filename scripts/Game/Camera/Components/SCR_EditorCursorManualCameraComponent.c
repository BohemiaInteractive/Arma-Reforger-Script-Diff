//! @ingroup ManualCamera

//! Check if cursor is not on top of another widget
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_EditorCursorManualCameraComponent : SCR_BaseManualCameraComponent
{
	protected SCR_MouseAreaEditorUIComponent m_MouseArea;
	protected SCR_MenuEditorComponent m_MenuManager;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!m_MouseArea)
			return;
		
		param.isCursorEnabled = m_MouseArea.IsMouseOn() || !m_MenuManager.IsVisible();
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		m_MenuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent, true));
		if (!m_MenuManager)
			return false;
		
		EditorMenuBase menu = m_MenuManager.GetMenu();
		if (!menu)
			return false;
		
		MenuRootComponent root = menu.GetRootComponent();
		if (!root)
			return false;
		
		m_MouseArea = SCR_MouseAreaEditorUIComponent.Cast(root.FindComponent(SCR_MouseAreaEditorUIComponent, true));
		return m_MouseArea != null;
	}
}
