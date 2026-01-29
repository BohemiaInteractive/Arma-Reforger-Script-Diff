/*
Class for tab menus, relies on a SCR_SuperMenuComponent placed on menu root widget
Do your tab related stuff in a child of SCR_SuperMenuComponent! Leave this for calling necessary events on it, and for global menu necessities
*/

class SCR_SuperMenuBase : MenuRootBase
{
	protected SCR_SuperMenuComponent m_SuperMenuComponent;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		m_SuperMenuComponent = SCR_SuperMenuComponent.FindComponent(GetRootWidget());
		if (!m_SuperMenuComponent)	
		{
			Print("SCR_SuperMenuBase | OnMenuOpen() | No SCR_SuperMenuComponent in layout root", LogLevel.ERROR);
			return;
		}

		m_SuperMenuComponent.Init(GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();

		if (m_SuperMenuComponent)
			m_SuperMenuComponent.OnMenuShow();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();

		if (m_SuperMenuComponent)
			m_SuperMenuComponent.OnMenuHide();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		if (m_SuperMenuComponent)
			m_SuperMenuComponent.OnMenuUpdate(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();

		if (m_SuperMenuComponent)
			m_SuperMenuComponent.OnMenuClose();
	}

	//----------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();

		if (m_SuperMenuComponent)
			m_SuperMenuComponent.OnMenuFocusGained();
	}
	
	//----------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();

		if (m_SuperMenuComponent)
			m_SuperMenuComponent.OnMenuFocusLost();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SuperMenuComponent GetSuperMenu()
	{
		return m_SuperMenuComponent;
	}
}