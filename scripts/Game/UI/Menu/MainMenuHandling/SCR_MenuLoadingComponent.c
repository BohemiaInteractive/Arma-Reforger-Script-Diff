// Used to handle custom inits of Configurable dialogs
enum SCR_EMenuLoadingAdditionalDialog
{
	SERVER_HOSTING = 0,
	MOD_DETAILS
}

class SCR_MenuLoadingComponent
{
	// Static saving values
	protected const string SAVE_LAST_MENU = "m_LastMenu";
	protected const string SAVE_ADDITIONAL_MENU = "m_LastAdditionalMenu"; // Can be used for extra menu e.g. Mod details
	protected const string SAVE_ADDITIONAL_DIALOG = "m_LastAdditionalDialog"; // Can be used for extra menu that is dialog e.g. Server hosting dialog

	// Variables
	protected static DialogUI m_DialogToDisplay = null;

	// Invoker
	static ref ScriptInvoker m_OnMenuOpening = new ScriptInvoker;

	//------------------------------------------------------------------------------------------------
	//! Load previously active menu instead of main menu screen
	static bool LoadLastMenu()
	{
		// Special cases
		if (JoinInvite())
			return false;

		// Open last menu
		string lastMenu = GameSessionStorage.s_Data[SAVE_LAST_MENU];
		if (lastMenu.IsEmpty() || (GetGame().m_bIsMainMenuOpen && lastMenu.ToInt() == ChimeraMenuPreset.MainMenu))
			return false;

		GetGame().GetMenuManager().OpenMenu(lastMenu.ToInt());

		// Additional menu
		string lastAdditionalMenu = GameSessionStorage.s_Data[SAVE_ADDITIONAL_MENU];
		if (!lastAdditionalMenu.IsEmpty())
			GetGame().GetMenuManager().OpenMenu(lastAdditionalMenu.ToInt());

		// Addition dialog
		string lastAdditionalDialog = GameSessionStorage.s_Data[SAVE_ADDITIONAL_DIALOG];
		if (!lastAdditionalDialog.IsEmpty())
			OpenAdditionalDialog(lastAdditionalDialog.ToInt());

		m_OnMenuOpening.Invoke(lastMenu.ToInt());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void SaveLastMenu(ChimeraMenuPreset menuPreset, ChimeraMenuPreset additionalMenu = -1, SCR_EMenuLoadingAdditionalDialog additionalDialog = -1)
	{
		GameSessionStorage.s_Data[SAVE_LAST_MENU] = menuPreset.ToString();

		if (additionalMenu != -1)
			GameSessionStorage.s_Data[SAVE_ADDITIONAL_MENU] = additionalMenu.ToString();

		if (additionalDialog != -1)
			GameSessionStorage.s_Data[SAVE_ADDITIONAL_DIALOG] = additionalDialog.ToString();
	}

	//------------------------------------------------------------------------------------------------
	static void ClearLastMenu()
	{
		GameSessionStorage.s_Data[SAVE_LAST_MENU] = string.Empty;
		GameSessionStorage.s_Data[SAVE_ADDITIONAL_MENU] = string.Empty;
		GameSessionStorage.s_Data[SAVE_ADDITIONAL_DIALOG] = string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Start joining to inveted room
	protected static bool JoinInvite()
	{
		ClientLobbyApi lobby = GetGame().GetBackendApi().GetClientLobby();

		// Check invited rooom
		Room invited = lobby.GetInviteRoom();
		if (!invited)
			return false;

		#ifdef SB_DEBUG
		Print("[SCR_MenuLoadingComponent] Client is invited to room: " + invited + ", name: " + invited.Name());
		#endif

		// Open server browser
		ServerBrowserMenuUI serverBrowser = ServerBrowserMenuUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu));
		if (serverBrowser)
		{
			serverBrowser.JoinProcess_Init(invited);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OpenAdditionalDialog(SCR_EMenuLoadingAdditionalDialog dialog)
	{
		switch (dialog)
		{
			case SCR_EMenuLoadingAdditionalDialog.SERVER_HOSTING: SCR_CommonDialogs.CreateServerHostingDialog(); break;
			//TODO: add other cases as they become necessary
		}
	}
}
