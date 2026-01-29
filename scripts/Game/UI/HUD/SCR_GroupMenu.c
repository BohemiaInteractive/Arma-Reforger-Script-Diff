class SCR_GroupMenu : SCR_SuperMenuBase
{
	protected static ref ScriptInvoker s_OnGroupMenuShow = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnGroupMenuShown()
	{
		return s_OnGroupMenuShow;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();		
		
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, GetRootWidget());
		if (cancel)
			cancel.m_OnActivated.Insert(Close);
		
		SCR_AIGroup.GetOnJoinPrivateGroupRequest().Insert(UpdateTabs);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(UpdateTabs);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();

		s_OnGroupMenuShow.Invoke();

		UpdateTabs();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{	
		super.OnMenuClose();
		
		SCR_AIGroup.GetOnJoinPrivateGroupRequest().Remove(UpdateTabs);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(UpdateTabs);
	}
	
	// TODO: move tab related stuff to super menu component and use an enum instead of magic numbers
	//------------------------------------------------------------------------------------------------
	void UpdateTabs()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		int playerID = GetGame().GetPlayerController().GetPlayerId();
		 
		SCR_AIGroup group = groupsManager.GetPlayerGroup(playerID);
		if (!group)
			return;		
		
		if (!groupsManager.IsPlayerInAnyGroup(playerID) || !group.IsPlayerLeader(playerID))
		{
			m_SuperMenuComponent.GetTabView().EnableTab(1, false, true);	
			return;
		}
		
		m_SuperMenuComponent.GetTabView().EnableTab(1, true, true);
			
		array<int> requesters = {};
		group.GetRequesterIDs(requesters);
				
		if (requesters.IsEmpty())
		{	
			m_SuperMenuComponent.GetTabView().ShowIcon(1, false);
			m_SuperMenuComponent.GetTabView().EnableTab(1, false, true);	
		}
		else
		{
			m_SuperMenuComponent.GetTabView().ShowIcon(1, true);	
			m_SuperMenuComponent.GetTabView().EnableTab(1, true, true);	
					
			Widget notificationIcon = m_SuperMenuComponent.GetTabView().GetEntryIcon(1);
			if (!notificationIcon)
				return;
			
			TextWidget text = TextWidget.Cast(notificationIcon.FindAnyWidget("NotificationCount"));
			if (!text)
				return;
			
			if (requesters.Count() > 99)
				text.SetText("99+");
			else			
				text.SetText(requesters.Count().ToString());
		}
	}
}
