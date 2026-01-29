class SCR_HUDMenuComponent : SCR_ScriptedWidgetComponent
{
	[Attribute(desc: "Identifier used by HudManager to find the correct layout. \nMUST BE UNIQUE!!")]
	protected string m_sIdentifier;

	protected ref SCR_HUDLayout m_HudLayout = new SCR_HUDLayout();

	protected SCR_HUDManagerLayoutHandler m_LayoutHandler;
	protected SCR_HUDManagerComponent m_HudManager;
	
	protected bool m_bIsMenuEnabled;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		if (!m_wRoot.IsEnabled())
			return;

		ActivateHudLayout();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (!m_LayoutHandler || !m_bIsMenuEnabled)
			return;

		m_bIsMenuEnabled = false;
		DeactivateHudLayout();
	}

	//------------------------------------------------------------------------------------------------
	//! Trigger a HUDLayout change when menu is opened
	protected void ActivateHudLayout()
	{
		if (!m_HudLayout)
		{
			Print("Failed to initialize SCR_HUDLayout!", LogLevel.WARNING);
			return;
		}

		m_HudLayout.SetRootWidget(m_wRoot);

		if (!m_sIdentifier)
		{
			Print("No Identifier set in " + ClassName() + ". Set a unique identifier!", LogLevel.WARNING);
			return;
		}

		m_HudLayout.SetIdentifier(m_sIdentifier);

		PlayerController pController = GetGame().GetPlayerController();
		if (!pController)
			return;

		m_HudManager = SCR_HUDManagerComponent.Cast(pController.FindComponent(SCR_HUDManagerComponent));
		if (!m_HudManager)
			return;

		m_LayoutHandler = SCR_HUDManagerLayoutHandler.Cast(m_HudManager.FindHandler(SCR_HUDManagerLayoutHandler));
		if (!m_LayoutHandler)
			return;

		if (m_HudManager.GetHUDElements().IsEmpty())
		{
			//Wait for the MainLayout to be initialize and try again
			m_LayoutHandler.GetOnMainLayoutInitialize().Insert(DelayHUDLayoutChange);
			return;
		}

		m_LayoutHandler.GetOnMainLayoutInitialize().Remove(DelayHUDLayoutChange);

		if (!m_LayoutHandler.GetAllHUDLayouts().Contains(m_HudLayout))
			m_LayoutHandler.AddHUDLayout(m_HudLayout);

		m_LayoutHandler.ChangeActiveHUDLayout(m_sIdentifier);
		
		m_bIsMenuEnabled = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Trigger a HUDLayout change when menu is closes
	protected void DeactivateHudLayout()
	{
		if (!m_LayoutHandler)
			return;

		//Remove the Listener in case the Menu gets closed before the MainLayout is initialized
		m_LayoutHandler.GetOnMainLayoutInitialize().Remove(DelayHUDLayoutChange);

		if (m_HudManager.GetHUDElements().IsEmpty())
			return;

		SCR_HUDLayout currentLayout = m_LayoutHandler.GetActiveLayout();

		//If there is no new layout yet, activate the Main layout otherwise the new Menu will Change it itself.
		if (currentLayout == m_HudLayout)
			m_LayoutHandler.ChangeActiveHUDLayout();

		if (m_LayoutHandler.GetAllHUDLayouts().Contains(m_HudLayout))
			m_LayoutHandler.RemoveHUDLayout(m_HudLayout);
	}

	//------------------------------------------------------------------------------------------------
	void DelayHUDLayoutChange()
	{
		//Delay the change of the HUDLayout when they just got initialized to prevent wrong reparenting
		GetGame().GetCallqueue().Call(ActivateHudLayout);
	}

	//------------------------------------------------------------------------------------------------
	//! Manualy enable HUDManager and force a change of active layout
	void EnableHUDMenu()
	{
		m_wRoot.SetEnabled(true);
		ActivateHudLayout();
	}

	//------------------------------------------------------------------------------------------------
	//! Manualy disable HUDManager and force a change of active layout
	void DisableHUDMenu()
	{
		m_bIsMenuEnabled = false;
		DeactivateHudLayout();
	}
}
