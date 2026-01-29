class SCR_BlockedUsersDialogUI : SCR_ConfigurableDialogUi
{
	protected const ResourceName ENTRY_WIDGET_NAME = "{3B2D99949BCE0542}UI/layouts/Menus/Dialogs/BlockedUsersEntry.layout";
	protected const string ENTRY_SCROLL_LIST = "Users";
	protected const ResourceName BLOCKED_USER_DIALOG_CONFIG = "{12C2EC09520BE302}Configs/Blocking/BlockedUsersDialog.conf";
	
	protected const string UNLOCK_BUTTON = "unblock";
	protected const string VIEW_GAMECARD_BUTTON = "viewGamecard";
	
	protected const string UNBLOCK_CONFIRM_MESSAGE = "#AR-Blocklist_Confirm";
	
	protected const string PLATFORM_
	
	protected int m_iMaxBlockedUserAmount = 10;
	
	protected Widget m_wUserListWidget;
	Widget m_wCurrentSelectedEntry;
	
	SCR_InputButtonComponent m_UnblockButton;
	protected SCR_InputButtonComponent m_GamecardButton;
	
	protected SocialComponent m_SocialComponent;
	
	protected ref BackendCallback m_BlocklistCallback;
	
	// Save the widget with the userID it belongs to, to get the correct ID from the focused widget
	ref map<Widget, BlockListItem> m_mUserList = new map<Widget, BlockListItem>();
	protected ref array<Widget> m_aUserList = {};
	
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		m_wUserListWidget = m_wRoot.FindAnyWidget(ENTRY_SCROLL_LIST);
		if (!m_wUserListWidget)
			return;
		
		ListBlockedUsers();
		
		m_UnblockButton = FindButton(UNLOCK_BUTTON);
		if (m_UnblockButton)
		{
			m_UnblockButton.m_OnActivated.Insert(UnblockAskConfirmation);
			m_UnblockButton.SetVisible(false, false);
		}
		
		// Show "View Gamecard" only for ps users
		m_GamecardButton = FindButton(VIEW_GAMECARD_BUTTON);
		if (m_GamecardButton)
			m_GamecardButton.SetVisible(false);
		
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputTypeChanged);
		
	}
		
	//----------------------------------------------------------------------------------------------
	protected void OnInputTypeChanged(EInputDeviceType old, EInputDeviceType newDevice)
	{
		if(!m_wCurrentSelectedEntry)
			return;
		
		SCR_BlockedUsersDialogEntryUIComponent selectedEntryComp = SCR_BlockedUsersDialogEntryUIComponent.Cast(m_wCurrentSelectedEntry.FindHandler(SCR_BlockedUsersDialogEntryUIComponent));
		if(!selectedEntryComp)
			return;
		
		switch(newDevice){
			case EInputDeviceType.GAMEPAD:
				selectedEntryComp.SetButtonsVisibility(false);
				m_UnblockButton.SetVisible(true, false);
				break;
			
			case EInputDeviceType.MOUSE:
				selectedEntryComp.SetButtonsVisibility(true);
				m_UnblockButton.SetVisible(false, false);
				break;
			
			case EInputDeviceType.KEYBOARD:
				selectedEntryComp.SetButtonsVisibility(false);
				m_UnblockButton.SetVisible(true, false);
				break;
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ListBlockedUsers()
	{
		GameBlocklist blocklist = GetGame().GetGameBlocklist();
		blocklist.OnBlockListUpdateInvoker.Insert(OnBlockedListUpdated);
		blocklist.UpdateBlockList();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBlockedListUpdated(bool success)
	{
		if (!success)
			return;
		
		SCR_WidgetHelper.RemoveAllChildren(m_wUserListWidget);
		
		array<BlockListItem> outItems = {};
		
		GameBlocklist blocklist = GetGame().GetGameBlocklist();
		blocklist.GetBlockedPlayers(outItems);
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget entry;
		RichTextWidget entryText;
		SCR_ModularButtonComponent button;
		SCR_BlockedUsersDialogEntryUIComponent entryComp;
		
		m_mUserList.Clear();
		
		foreach(BlockListItem item : outItems)
		{
			if (!item)
				continue;
			
			entry = workspace.CreateWidgets(ENTRY_WIDGET_NAME, m_wUserListWidget);
			if (!entry)
				continue;
			
			entryComp = SCR_BlockedUsersDialogEntryUIComponent.Cast(entry.FindHandler(SCR_BlockedUsersDialogEntryUIComponent));
			if (!entryComp)
				continue;
			
			entryComp.SetPlatfrom(item.GetPlatform());
			entryComp.SetPlayerName(item.GetName());
			entryComp.GetProfileButton().m_OnClicked.Insert(OnViewGamecard);
			entryComp.GetUnblockButton().m_OnClicked.Insert(UnblockAskConfirmation);
			
			// Save the widget & player ID
			m_mUserList.Insert(entry, item);
			
			button = SCR_ModularButtonComponent.FindComponent(entry);
			if (button){
				button.m_OnFocus.Insert(OnEntryFocused);
				button.m_OnFocusLost.Insert(OnEntryFocusLost);
			}
			
		}
	}
		
	//------------------------------------------------------------------------------------------------
	protected void OnEntryFocused(SCR_ModularButtonComponent modularButton)
	{
		m_wCurrentSelectedEntry = modularButton.GetRootWidget();
		
		SCR_BlockedUsersDialogEntryUIComponent comp = SCR_BlockedUsersDialogEntryUIComponent.Cast(m_wCurrentSelectedEntry.FindHandler(SCR_BlockedUsersDialogEntryUIComponent));
		if (!comp)
			return;
		
		if(GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
		{
			comp.SetButtonsVisibility(true);
			if (m_UnblockButton)
				m_UnblockButton.SetVisible(false, false);
		}
		else
		{
			comp.SetButtonsVisibility(false);
			if (m_UnblockButton)
				m_UnblockButton.SetVisible(true, false);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntryFocusLost(SCR_ModularButtonComponent modularButton)
	{
		m_wCurrentSelectedEntry = modularButton.GetRootWidget();
		if(!m_wCurrentSelectedEntry)
			return;
		
		SCR_BlockedUsersDialogEntryUIComponent comp = SCR_BlockedUsersDialogEntryUIComponent.Cast(m_wCurrentSelectedEntry.FindHandler(SCR_BlockedUsersDialogEntryUIComponent));
		if (!comp)
			return;
		
		comp.SetButtonsVisibility(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnblockAskConfirmation()
	{
		string playerName = SCR_BlockedUsersDialogEntryUIComponent.Cast(m_wCurrentSelectedEntry.FindHandler(SCR_BlockedUsersDialogEntryUIComponent)).GetPlayerName();
		
		SCR_ConfigurableDialogUi dialog = new SCR_ConfigurableDialogUi();
		SCR_ConfigurableDialogUi.CreateFromPreset(BLOCKED_USER_DIALOG_CONFIG, "unblock_player_confirm", dialog);
		
		//dialog.SetMessage(string.Format(UNBLOCK_CONFIRM_MESSAGE, playerName));
		TextWidget txtWidget = dialog.GetMessageWidget();
		txtWidget.SetTextFormat(UNBLOCK_CONFIRM_MESSAGE, playerName);
		
		dialog.m_OnConfirm.Insert(OnUserUnblock);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnUserUnblock()
	{	
		BlockListItem userItem;
		
		m_mUserList.Find(m_wCurrentSelectedEntry, userItem);
		
		if (!userItem)
			return;
		
		m_BlocklistCallback = new BackendCallback();
		m_BlocklistCallback.SetOnSuccess(OnBlocklistRequestSuccess);
		m_BlocklistCallback.SetOnError(OnBlocklistRequestError);
		userItem.DeleteBlock(m_BlocklistCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnViewGamecard()
	{	
		//TODO: Fill in with view profile functionality once it's all implemented.
		//GetGame().GetPlayerManager().ShowUserProfile(m_wCurrentSelectedEntry.GetUserID());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		if (m_wUserListWidget)
		{
			SCR_WidgetHelper.RemoveAllChildren(m_wUserListWidget);
		}
		GetGame().OnInputDeviceUserChangedInvoker().Remove(OnInputTypeChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBlocklistRequestSuccess()
	{
		m_wCurrentSelectedEntry.RemoveFromHierarchy();
		m_mUserList.Remove(m_wCurrentSelectedEntry);
		m_wCurrentSelectedEntry = null;
		m_UnblockButton.SetEnabled(false);
		GetGame().GetGameBlocklist().UpdateBlockList();
	}

	//------------------------------------------------------------------------------------------------
	void OnBlocklistRequestError()
	{
		SCR_BlockedUsersDialogUI dialog = new SCR_BlockedUsersDialogUI();
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_AccountWidgetComponent.BLOCKED_USER_DIALOG_CONFIG, "block_failed_general", dialog);
	}
}
