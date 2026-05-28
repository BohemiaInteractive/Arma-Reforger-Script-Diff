class SCR_GroupSubMenu : SCR_GroupSubMenuBase
{
	protected Faction m_PlayerFaction;
	protected SCR_ChatPanel m_ChatPanelComponent;
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			m_PlayerFaction = SCR_Faction.Cast(factionManager.GetLocalPlayerFaction());
		else
			m_PlayerFaction = null;
		
		Widget chatPanel = menuRoot.FindAnyWidget("ChatPanel");
		if (!chatPanel)
			return;
		 
		m_ChatPanelComponent = SCR_ChatPanel.Cast(chatPanel.FindHandler(SCR_ChatPanel));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();
		
		if (m_ChatPanelComponent)
		{
			m_ChatPanelComponent.GetOnChatOpen().Insert(OnChatOpen);
			m_ChatPanelComponent.GetOnChatClosed().Insert(OnChatClosed);
			
			if (m_ChatPanelComponent.IsOpen())
				OnChatOpen();
			
			if (m_ChatPanelComponent.GetFadeOut() == false)
			{
				Widget chatContent = m_wMenuRoot.FindAnyWidget("ChatContent");
				if (chatContent)
					chatContent.SetVisible(true);
				
				SCR_FadeInOutAnimator chatAnimator = m_ChatPanelComponent.GetFadeInOutAnimator();
				if (chatAnimator)
					chatAnimator.GetOnStateChanged().Insert(OnAnimatorStateChanged);
			}
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnTabHide() 
	{ 
		super.OnTabHide();
		
		if (m_ChatPanelComponent)
		{
			m_ChatPanelComponent.GetOnChatOpen().Remove(OnChatOpen);
			m_ChatPanelComponent.GetOnChatClosed().Remove(OnChatClosed);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnChatOpen()
	{
		Widget chatContent = m_wMenuRoot.FindAnyWidget("ChatContent");
		if (chatContent)
			chatContent.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnChatClosed()
	{
		if (SCR_ChatPanelManager.GetInstance().GetMessages().Count() == 0)
		{
			Widget chatContent = m_wMenuRoot.FindAnyWidget("ChatContent");
			if (chatContent)
				chatContent.SetVisible(false);
			
			return;
		}
		
		SCR_FadeInOutAnimator chatAnimator = m_ChatPanelComponent.GetFadeInOutAnimator();
		if (!chatAnimator)
			return;
		
		chatAnimator.GetOnStateChanged().Insert(OnAnimatorStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAnimatorStateChanged(int formerState, int actualState)
	{
		if (formerState == 2 && actualState == 3)
		{
			Widget chatContent = m_wMenuRoot.FindAnyWidget("ChatContent");
			if (chatContent)
				chatContent.SetVisible(false);
			
			m_ChatPanelComponent.GetFadeInOutAnimator().GetOnStateChanged().Remove(OnAnimatorStateChanged);
		}	
	}
}
