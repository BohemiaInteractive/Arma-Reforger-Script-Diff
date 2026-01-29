//------------------------------------------------------------------------------------------------
//! Fullscreen map menu
class SCR_MapMenuUI: ChimeraMenuBase
{	
	protected SCR_MapEntity m_MapEntity;	
	protected SCR_ChatPanel m_ChatPanel;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{	
		if (m_MapEntity)
		{	
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
				return;
		
			SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(gameMode.FindComponent(SCR_MapConfigComponent));
			if (!configComp)
				return;
		
			MapConfiguration mapConfigFullscreen = m_MapEntity.SetupMapConfig(EMapEntityMode.FULLSCREEN, configComp.GetGadgetMapConfig(), GetRootWidget());
			m_MapEntity.OpenMap(mapConfigFullscreen);
		}
		
		Widget wChatPanel = GetRootWidget().FindAnyWidget("ChatPanel");
		if (wChatPanel)
			m_ChatPanel = SCR_ChatPanel.Cast(wChatPanel.FindHandler(SCR_ChatPanel));
		
		GetGame().GetInputManager().AddActionListener("ChatToggle", EActionTrigger.DOWN, Callback_OnChatToggleAction);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{		
		if (m_MapEntity)
			m_MapEntity.CloseMap();
		
		GetGame().GetInputManager().RemoveActionListener("ChatToggle", EActionTrigger.DOWN, Callback_OnChatToggleAction);
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnMenuInit()
	{		
		if (!m_MapEntity)
			m_MapEntity = SCR_MapEntity.GetMapInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		if (m_ChatPanel)
			m_ChatPanel.OnUpdateChat(tDelta);
	}
	
	//------------------------------------------------------------------------------------------------
	void Callback_OnChatToggleAction()
	{
		if (!m_ChatPanel)
			return;
		
		if (!m_ChatPanel.IsOpen())
			SCR_ChatPanelManager.GetInstance().OpenChatPanel(m_ChatPanel);
	}
};