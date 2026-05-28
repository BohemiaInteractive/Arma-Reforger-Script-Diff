class SCR_RequestToJoinSubmenu : SCR_SubMenuBase
{		
	protected ref ScriptInvokerVoid m_OnJoinRequestRespond;
	protected ref array<Widget> m_aEntryWidgets = {};
	protected VerticalLayoutWidget m_wContent;

	protected const string REQUESTER_ENTRY_LAYOUT = "{B3381965FF7747CE}UI/layouts/Menus/GroupSlection/GroupRequestEntry.layout";
	protected const string INPUT_CONTEXT_NAME = "GroupMenuContext";
	protected const string CONTENT_HOLDER_WIDGET = "Content";
	protected const string PLAYER_NAME_WIDGET = "PlayerName";
	protected const string PLATFORM_ICON_WIDGET = "PlatformImage";
				
	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();	
		
		UpdateRequesters();
		
		GetOnJoinRequestRespond().Insert(UpdateRequesters);
		SCR_AIGroup.GetOnJoinPrivateGroupRequest().Insert(UpdateRequesters);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabRemove()
	{
		super.OnTabRemove();
		
		GetOnJoinRequestRespond().Remove(UpdateRequesters);
		SCR_AIGroup.GetOnJoinPrivateGroupRequest().Remove(UpdateRequesters);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		GetGame().GetInputManager().ActivateContext(INPUT_CONTEXT_NAME);
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
		
		VerticalLayoutWidget content = VerticalLayoutWidget.Cast(m_wMenuRoot.FindAnyWidget(CONTENT_HOLDER_WIDGET));
		if (!content)
			return;				
		
		for (int i = 0, count = m_aEntryWidgets.Count(); i < count ;i++)
		{
			content.RemoveChild(m_aEntryWidgets[i]);
		}
		
		m_aEntryWidgets.Clear();
		
		array<int> requesterIDs = {};
		group.GetRequesterIDs(requesterIDs);	

		ButtonWidget focusedElement;
		Widget entryWidget;
		TextWidget playerName;
		ImageWidget platformIcon;
		SCR_JoinRequestEntry entryMenu;
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		foreach (int requesterId : requesterIDs)
		{
			entryWidget = GetGame().GetWorkspace().CreateWidgets(REQUESTER_ENTRY_LAYOUT, content);
			if (!entryWidget)
				continue;
			
			playerName = TextWidget.Cast(entryWidget.FindAnyWidget(PLAYER_NAME_WIDGET));
			if (!playerName)
				continue;
			
			platformIcon = ImageWidget.Cast(entryWidget.FindAnyWidget(PLATFORM_ICON_WIDGET));
			
			entryMenu = SCR_JoinRequestEntry.Cast(entryWidget.FindHandler(SCR_JoinRequestEntry));
			if (!entryMenu)
				continue;
			
			if (!focusedElement)
				focusedElement = ButtonWidget.Cast(entryWidget);

			m_aEntryWidgets.Insert(entryWidget);

			entryMenu.SetParentMenu(this);
			entryMenu.SetPlayerID(requesterId);
			entryMenu.Init(entryWidget);
			
			playerName.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(requesterId));
			
			if (platformIcon && playerController)
				playerController.SetPlatformImageTo(requesterId, platformIcon);
		}

		GetGame().GetWorkspace().SetFocusedWidget(focusedElement);

		// TODO: a sub menu tab should not call a method on the menu class. This should be an invoker
		SCR_GroupMenu groupMenu = SCR_GroupMenu.Cast(ChimeraMenuBase.GetOwnerMenu(GetRootWidget()));
		if (!groupMenu)
			return;
		
		groupMenu.UpdateTabs();
	}
	
	//------------------------------------------------------------------------------------------------
	void InvokeOnJoinRequestRespond()
	{
		if (m_OnJoinRequestRespond)
			m_OnJoinRequestRespond.Invoke();
	}
}
