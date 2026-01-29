class SCR_BlockedPlayerPresentDialog : SCR_ConfigurableDialogUi
{
	protected const string TEXT_ID = "#AR-ServerBrowser_JoinBlockedPlayer";
	protected const string SEPARATOR = ", ";
	
	//------------------------------------------------------------------------------------------------
	void SCR_BlockedPlayerPresentDialog(array<BlockedRoomPlayer> blockedPlayers)
	{
		SCR_BlockedPlayerPresentDialog dialog = SCR_BlockedPlayerPresentDialog.Cast(SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "blocked_player_present", this));
		dialog.Init(blockedPlayers);
	}
	
	//------------------------------------------------------------------------------------------------
	void Init(array<BlockedRoomPlayer> blockedPlayers)
	{
		string blockedPlayersNames;
		foreach (BlockedRoomPlayer player : blockedPlayers)
		{
			if (!blockedPlayersNames.IsEmpty())
				blockedPlayersNames += SEPARATOR;
			
			blockedPlayersNames += player.GetName();
		}
		
		TextWidget message = GetMessageWidget();
		if (message)
			message.SetTextFormat(TEXT_ID, blockedPlayersNames);
	}
}