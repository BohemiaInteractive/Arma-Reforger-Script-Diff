/*!
	This component serves as a config for deploy menu elements.
	Set the names of widgets where request handlers are attached to in the deploy menu layout.
	Has to be attached at the root of the deploy menu layout.
*/
class SCR_DeployMenuHandler : SCR_ScriptedWidgetComponent
{
	[Attribute("FactionOverlay")]
	protected string m_sFactionUIHandler;

	[Attribute("LoadoutSelector")]
	protected string m_sLoadoutUIHandler;

	[Attribute("GroupSelector")]
	protected string m_sGroupUIHandler;

	[Attribute("SpawnPointSelector")]
	protected string m_sSpawnPointUIHandler;
	
	[Attribute("FactionPlayerList")]
	protected string m_sFactionPlayerList;

	[Attribute("GroupPlayerList")]
	protected string m_sGroupPlayerList;
	
	[Attribute("HudManagerLayout")]
	protected string m_sHudLayoutUIHandler;
	
	[Attribute(desc:"Determines which widgets should be hidden when opening the pause menu")]
	protected ref array<string> m_aHiddenWidgetsOnPause;

	[Attribute(desc:"Determines which widgets should be disabled when opening the pause menu")]
	protected ref array<string> m_aDisabledWidgetsOnPause;
	
	protected ref array<Widget> m_aHiddenWidgets = {};
	protected ref array<Widget> m_aDisabledWidgets = {};
	
	protected SCR_FactionRequestUIComponent m_FactionRequestHandler;
	protected SCR_LoadoutRequestUIComponent m_LoadoutRequestHandler;
	protected SCR_GroupRequestUIComponent m_GroupRequestHandler;
	protected SCR_SpawnPointRequestUIComponent m_SpawnPointUIHandler;
	protected SCR_FactionPlayerList m_FactionPlayerList;	
	protected SCR_GroupPlayerList m_GroupPlayerList;
	protected SCR_HUDMenuComponent m_HudMenuComponent;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		PauseMenuUI.m_OnPauseMenuOpened.Insert(OnPauseMenuOpened);
		PauseMenuUI.m_OnPauseMenuClosed.Insert(OnPauseMenuClosed);
		
		foreach (string name : m_aHiddenWidgetsOnPause)
		{
			Widget widget = m_wRoot.FindAnyWidget(name);
			
			if (widget)
				m_aHiddenWidgets.Insert(widget);
		}
		
		foreach (string name : m_aDisabledWidgetsOnPause)
		{
			Widget widget = m_wRoot.FindAnyWidget(name);
			
			if (widget)
				m_aDisabledWidgets.Insert(widget);
		}
		
		Widget hudMenu = m_wRoot.FindAnyWidget(m_sHudLayoutUIHandler);
		if (hudMenu)
			m_HudMenuComponent = SCR_HUDMenuComponent.Cast(hudMenu.FindHandler(SCR_HUDMenuComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		PauseMenuUI.m_OnPauseMenuOpened.Remove(OnPauseMenuOpened);
		PauseMenuUI.m_OnPauseMenuClosed.Remove(OnPauseMenuClosed);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPauseMenuOpened()
	{
		UpdateWidgetsOnPause(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPauseMenuClosed()
	{
		UpdateWidgetsOnPause(false);
		ForceHUDLayout();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateWidgetsOnPause(bool paused)
	{
		foreach (Widget widget : m_aHiddenWidgets)
		{
			widget.SetVisible(!paused);
		}
		
		foreach (Widget widget : m_aDisabledWidgets)
		{
			widget.SetEnabled(!paused);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ForceHUDLayout()
	{
		if (!m_HudMenuComponent)
			return;
		
		m_HudMenuComponent.EnableHUDMenu();
	}

	//------------------------------------------------------------------------------------------------
	SCR_FactionRequestUIComponent GetFactionRequestHandler()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sFactionUIHandler);
		if (!tmp)
			return null;

		return SCR_FactionRequestUIComponent.Cast(tmp.FindHandler(SCR_FactionRequestUIComponent));
	}

	//------------------------------------------------------------------------------------------------
	SCR_LoadoutRequestUIComponent GetLoadoutRequestHandler()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sLoadoutUIHandler);
		if (!tmp)
			return null;

		return SCR_LoadoutRequestUIComponent.Cast(tmp.FindHandler(SCR_LoadoutRequestUIComponent));
	}

	//------------------------------------------------------------------------------------------------
	SCR_GroupRequestUIComponent GetGroupRequestHandler()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sGroupUIHandler);
		if (!tmp)
			return null;

		return SCR_GroupRequestUIComponent.Cast(tmp.FindHandler(SCR_GroupRequestUIComponent));
	}

	//------------------------------------------------------------------------------------------------
	SCR_SpawnPointRequestUIComponent GetSpawnPointRequestHandler()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sSpawnPointUIHandler);
		if (!tmp)
			return null;

		return SCR_SpawnPointRequestUIComponent.Cast(tmp.FindHandler(SCR_SpawnPointRequestUIComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_FactionPlayerList GetFactionPlayerList()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sFactionPlayerList);
		if (!tmp)
			return null;
		
		return SCR_FactionPlayerList.Cast(tmp.FindHandler(SCR_FactionPlayerList));
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_GroupPlayerList GetGroupPlayerList()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sGroupPlayerList);
		if (!tmp)
			return null;
		
		return SCR_GroupPlayerList.Cast(tmp.FindHandler(SCR_GroupPlayerList));	
	}
}