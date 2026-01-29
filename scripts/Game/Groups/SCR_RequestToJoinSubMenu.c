class SCR_RequestToJoinSubmenu : SCR_SubMenuBase
{		
	protected const string REQUESTER_ENTRY_LAYOUT = "{B3381965FF7747CE}UI/layouts/Menus/GroupSlection/GroupRequestEntry.layout";
	protected ref ScriptInvokerVoid m_OnJoinRequestRespond;
	protected ref array<Widget> m_aEntryWidgets = {};
	protected VerticalLayoutWidget m_wContent;
				
	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();	
		
		UpdateRequesters();
		
		GetOnJoinRequestRespond().Insert(UpdateRequesters);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabRemove()
	{
		super.OnTabRemove();
		
		GetOnJoinRequestRespond().Remove(UpdateRequesters);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnJoinRequestRespond()
	{
		if (!m_OnJoinRequestRespond)
			m_OnJoinRequestRespond = new ScriptInvokerVoid();
		
		return m_OnJoinRequestRespond;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateRequesters()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.GetPlayerGroup(GetGame().GetPlayerController().GetPlayerId());
		if (!group)
			return;
		
		VerticalLayoutWidget content = VerticalLayoutWidget.Cast(m_wMenuRoot.FindAnyWidget("Content"));
		if (!content)
			return;				
		
		for (int i = 0, count = m_aEntryWidgets.Count(); i < count ;i++)
		{
			content.RemoveChild(m_aEntryWidgets[i]);
		}
		
		m_aEntryWidgets.Clear();
		
		array<int> requesterIDs = {};
		
		group.GetRequesterIDs(requesterIDs);	
					
		for (int i = 0, count = requesterIDs.Count(); i < count ;i++)
		{
			Widget entryWidget = GetGame().GetWorkspace().CreateWidgets(REQUESTER_ENTRY_LAYOUT, content);
			if (!entryWidget)
				continue;
			
			TextWidget playerName = TextWidget.Cast(entryWidget.FindAnyWidget("PlayerName"));
			if (!playerName)
				continue;
			
			ImageWidget platformIcon = ImageWidget.Cast(entryWidget.FindAnyWidget("PlatformImage"));
			
			ButtonWidget refuseWidget = ButtonWidget.Cast(entryWidget.FindAnyWidget("Refuse"));
			if (!refuseWidget)
				continue;
			
			ButtonWidget acceptWidget = ButtonWidget.Cast(entryWidget.FindAnyWidget("Accept"));
			if (!acceptWidget)
				continue;
			
			SCR_InputButtonComponent refuseButton = SCR_InputButtonComponent.Cast(refuseWidget.FindHandler(SCR_InputButtonComponent));
			if (!refuseButton)
				continue;
			
			SCR_InputButtonComponent acceptButton = SCR_InputButtonComponent.Cast(acceptWidget.FindHandler(SCR_InputButtonComponent));
			if (!acceptButton)
				continue;
			
			SCR_JoinRequestEntry entry = SCR_JoinRequestEntry.Cast(entryWidget.FindHandler(SCR_JoinRequestEntry));
			if (!entry)
				continue;
			
			m_aEntryWidgets.Insert(entryWidget);
			
			entry.SetPlayerID(requesterIDs[i]);
			
			playerName.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(requesterIDs[i]));
			
			acceptButton.m_OnClicked.Insert(AcceptToJoinPrivateGroup);
			refuseButton.m_OnClicked.Insert(RefuseJoinPrivateGroup);
			
			if (platformIcon)
			{
				SCR_PlayerController playerCtrl = SCR_PlayerController.Cast(GetGame().GetPlayerController());
				if (playerCtrl)
					playerCtrl.SetPlatformImageTo(requesterIDs[i], platformIcon);
			}	
		}		
		
		// TODO: a sub menu tab should not call a method on the menu class. This should be an invoker
		SCR_GroupMenu groupMenu = SCR_GroupMenu.Cast(ChimeraMenuBase.GetOwnerMenu(GetRootWidget()));
		if (!groupMenu)
			return;
		
		groupMenu.UpdateTabs();
	}	
	
	//------------------------------------------------------------------------------------------------
	void AcceptToJoinPrivateGroup()
	{			
		Widget widgetEntry = GetRootWidget().FindAnyWidget("RequesterEntry");
		if (!widgetEntry)
			return;
		
		SCR_JoinRequestEntry entry = SCR_JoinRequestEntry.Cast(widgetEntry.FindHandler(SCR_JoinRequestEntry));
		if (!entry)
			return;
	
		SCR_PlayerControllerGroupComponent playerGroupComponent = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(GetGame().GetPlayerController().GetPlayerId());
		if (!playerGroupComponent)
			return;
				
		playerGroupComponent.AcceptJoinPrivateGroup(entry.GetPlayerID(), true);				
		
		GetGame().GetCallqueue().CallLater(InvokeOnJoinRequestRespond, 200); //call later because requesters are updated before array is clear
	}	
	
	//------------------------------------------------------------------------------------------------
	void RefuseJoinPrivateGroup()
	{
		Widget widgetEntry = GetRootWidget().FindAnyWidget("RequesterEntry");
		if (!widgetEntry)
			return;
		
		SCR_JoinRequestEntry entry = SCR_JoinRequestEntry.Cast(widgetEntry.FindHandler(SCR_JoinRequestEntry));
		if (!entry)
			return;
		
		SCR_PlayerControllerGroupComponent playerGroupComponent = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(GetGame().GetPlayerController().GetPlayerId());
		if (!playerGroupComponent)
			return;
		
		playerGroupComponent.AcceptJoinPrivateGroup(entry.GetPlayerID(), false);
		
		GetGame().GetCallqueue().CallLater(InvokeOnJoinRequestRespond, 200); //call later because requesters are updated before array is clear
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeOnJoinRequestRespond()
	{
		if (m_OnJoinRequestRespond)
			m_OnJoinRequestRespond.Invoke();
	}
}