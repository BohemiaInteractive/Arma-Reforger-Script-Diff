// Ban Subcommands Args
enum SCR_EBanSubcommandArg
{
	EBSA_MISSING,
	EBSA_HELP,
	EBSA_CREATE,
	EBSA_REMOVE,
	EBSA_LIST,
	EBSA_LIST_FILTERED,
	EBSA_END
}

// Ban Create Args
enum SCR_EBanCreateArgs
{
	EBCA_COMMAND = 0,
	EBCA_SUBCOMMAND,
	EBCA_ID,
	EBCA_DURATION,
	EBCA_REASON,
	EBCA_END
}

// Ban Remove Args
enum SCR_EBanRemoveArgs
{
	EBRA_COMMAND = 0,
	EBRA_SUBCOMMAND,
	EBRA_ID,
	EBRA_END
}

// Ban List Args
enum SCR_EBanListArgs
{
	EBLA_COMMAND = 0,
	EBLA_SUBCOMMAND,
	EBLA_PAGE,
	EBLA_END
}

enum SCR_EPlayerIdType
{
	EPIT_IDENTITY = 0,
	EPIT_PLAYERID,
	EPIT_USERNAME
}

// Ban Server Command
class BanCommand: ScrServerCommand
{
	protected ref StateBackendCallback m_Callback;
	protected ref BanListPageParams m_Params;
	protected BanServiceApi m_BanApi;
	protected SCR_EBanSubcommandArg m_eSubcommandArg;
	
	const static int BAN_LIST_ITEMS_PER_PAGE_CHAT = 10;
	const static int BAN_LIST_ITEMS_PER_PAGE_RCON = 25;
	
	protected int m_iBanPlayerId;
	protected int m_iDuration;
	protected int m_iPage;
	protected string m_sBanReason = "";
	protected string m_sPlayerName = "";
	
	// Handle Create Subcommand
	//-----------------------------------------------------------------------------
	protected ScrServerCmdResult CreateBan(array<string> argv, int playerId)
	{	
		// All args except Reason are required
		if (argv.Count() < SCR_EBanCreateArgs.EBCA_REASON)
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.PARAMETERS);

		argv = TryParsePlayerNameWithSpaces(argv, SCR_EBanCreateArgs.EBCA_ID);

		// Get duration for ban in Int from String
		m_iDuration = argv[SCR_EBanCreateArgs.EBCA_DURATION].ToInt(-1);
		if (m_iDuration < 0)
			return ScrServerCmdResult("Not Valid time value in seconds", EServerCmdResultType.ERR);
	
		// If there are some additional args then combine them into reason string
		m_sBanReason = "";
		if (argv.Count() > SCR_EBanCreateArgs.EBCA_REASON)
		{
			for (int i = SCR_EBanCreateArgs.EBCA_REASON, count = argv.Count() - 1; i < count; i++)
			{
				m_sBanReason += argv[i] + " ";
			}
			m_sBanReason += argv[argv.Count() - 1];
		}
		else
		{
			m_sBanReason = "No reason provided" 
		}
	
		// Create Ban for player based on the id type (identity, playerId or username)
		m_iBanPlayerId = 0;
		m_sPlayerName = "";
		
		SCR_EPlayerIdType idType = GetPlayerIdType(argv[SCR_EBanCreateArgs.EBCA_ID]);
		switch (idType)
		{
			case SCR_EPlayerIdType.EPIT_PLAYERID:
			{
				m_iBanPlayerId = argv[SCR_EBanCreateArgs.EBCA_ID].ToInt();
				m_sPlayerName = GetGame().GetPlayerManager().GetPlayerName(m_iBanPlayerId);
				if (m_sPlayerName == "")
					return ScrServerCmdResult("Player not found", EServerCmdResultType.ERR);
				PrintFormat("Request Ban %1[%2] for %3 seconds. Reason: '%4'", m_sPlayerName, m_iBanPlayerId, m_iDuration, m_sBanReason);
				bool success = m_BanApi.CreateBanPlayerId(m_Callback, m_iBanPlayerId, m_sBanReason, m_iDuration);
				if (!success)
					return ScrServerCmdResult("Failed to create ban.", EServerCmdResultType.ERR);			
			} break;

			case SCR_EPlayerIdType.EPIT_IDENTITY:
			{
				m_sPlayerName = argv[SCR_EBanCreateArgs.EBCA_ID];
				PrintFormat("Request Ban %1 for %2 seconds. Reason: '%3'", m_sPlayerName, m_iDuration, m_sBanReason);
				bool success = m_BanApi.CreateBanIdentityId(m_Callback, m_sPlayerName, m_sBanReason, m_iDuration);
				if (!success)
					return ScrServerCmdResult("Failed to create ban.", EServerCmdResultType.ERR);
			} break;

			case SCR_EPlayerIdType.EPIT_USERNAME:
			{
				m_sPlayerName = argv[SCR_EBanCreateArgs.EBCA_ID];
				m_iBanPlayerId = GetPlayerIdFromName(m_sPlayerName);
				PrintFormat("Request Ban %1[%2] for %3 seconds. Reason: '%4'", m_sPlayerName, m_iBanPlayerId, m_iDuration, m_sBanReason);
				bool success = m_BanApi.CreateBanPlayerId(m_Callback, m_iBanPlayerId, m_sBanReason, m_iDuration);
				if (!success)
					return ScrServerCmdResult("Failed to create ban.", EServerCmdResultType.ERR);
			} break;
		}

		return ScrServerCmdResult(string.Empty, EServerCmdResultType.PENDING);
	}
	
	// Handle Remove Subcommand
	//-----------------------------------------------------------------------------
	protected ScrServerCmdResult RemoveBan(array<string> argv, int playerId)
	{
		// All args are required
		if (argv.Count() < SCR_EBanRemoveArgs.EBRA_END)
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.PARAMETERS);

		argv = TryParsePlayerNameWithSpaces(argv, SCR_EBanRemoveArgs.EBRA_ID);

		bool success = false;
		SCR_EPlayerIdType idType = GetPlayerIdType(argv[SCR_EBanRemoveArgs.EBRA_ID]);
		if (idType == SCR_EPlayerIdType.EPIT_USERNAME)
		{
			m_Callback.m_eLastMatchType = EStringMatchType.ESMT_EQUALS;
			m_sPlayerName = argv[SCR_EBanRemoveArgs.EBRA_ID];
			success = m_BanApi.RemoveBanByName(m_Callback, m_sPlayerName, EStringMatchType.ESMT_EQUALS);
			PrintFormat("Requested unban of user with user name: '%1'", argv[SCR_EBanRemoveArgs.EBRA_ID]);
		}
		else
		{
			success = m_BanApi.RemoveBans(m_Callback, {argv[SCR_EBanRemoveArgs.EBRA_ID]});
			PrintFormat("Requested unban of user with identity: '%1'", argv[SCR_EBanRemoveArgs.EBRA_ID]); 
		}

		if (!success)
			return ScrServerCmdResult("Failed to remove ban.", EServerCmdResultType.ERR);
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.PENDING);
	}
	
	// Handle List Subcommand
	//-----------------------------------------------------------------------------
	protected ScrServerCmdResult ListBans(array<string> argv, int playerId)
	{
		// Get which page was requested. Show first page if unspecified
		if (argv.Count() < SCR_EBanListArgs.EBLA_END)
		{
			m_iPage = 1;
		}
		else
		{
			m_iPage = argv[SCR_EBanListArgs.EBLA_PAGE].ToInt();
			if (m_iPage < 1)
				return ScrServerCmdResult("Not Valid page number", EServerCmdResultType.ERR);
		}
		
		m_Params = new BanListPageParams();
		
		// Set different page limits for chat and RCON
		if (playerId == 0)
			m_Params.limit = BAN_LIST_ITEMS_PER_PAGE_RCON;
		else
			m_Params.limit = BAN_LIST_ITEMS_PER_PAGE_CHAT;
		m_Params.offset = m_Params.limit * (m_iPage - 1);
		bool success = m_BanApi.RequestServerBanList(m_Callback, m_Params);
		if (!success)
			return ScrServerCmdResult("Failed to list bans.", EServerCmdResultType.ERR);
		return ScrServerCmdResult("Processing ban list", EServerCmdResultType.PENDING);
	}
	
	// Handle result for List Subcommand
	//-----------------------------------------------------------------------------
	protected ScrServerCmdResult ListBansResult()
	{
		// Check if there are any bans
		if (m_BanApi.GetPageCount() == 0)
			return ScrServerCmdResult("Server has no bans to list.", EServerCmdResultType.OK);
		
		// Error if requested page exceeded page count
		if ((m_BanApi.GetPage() + 1) > m_BanApi.GetPageCount())
			return ScrServerCmdResult("Page not found!", EServerCmdResultType.ERR);
		
		// Generate string resposne for list
		ref array<OnlineBanListData> banList = new array<OnlineBanListData>;
		m_BanApi.GetPageItems(banList);
		string banListStr = "Total bans: " + m_BanApi.GetTotalItemCount() + " | Page: " + (m_BanApi.GetPage() + 1) +"/" + m_BanApi.GetPageCount() + "\n";
		banListStr += "- Identity Id | Banned name\n";
		foreach (OnlineBanListData ban : banList)
		{
			banListStr += string.Format("- %1 | %2\n", ban.identityId, ban.bannedName);
		}
	
		return ScrServerCmdResult(banListStr, EServerCmdResultType.OK);
	}

	// Handle result for multiple target match
	//-----------------------------------------------------------------------------	
	protected ScrServerCmdResult ListBansResultFiltered()
	{
		// Check if there are any bans
		if (m_BanApi.GetPageCount() == 0)
			return ScrServerCmdResult("Server has no bans to list.", EServerCmdResultType.OK);
		
		// Error if requested page exceeded page count
		if ((m_BanApi.GetPage() + 1) > m_BanApi.GetPageCount())
			return ScrServerCmdResult("Page not found!", EServerCmdResultType.ERR);
		
		// Generate string resposne for list
		ref array<OnlineBanListData> banList = new array<OnlineBanListData>;
		m_BanApi.GetPageItems(banList);
		string banListStr = "Multiple users found! Please use a unique identifier.\n";
		banListStr += "Total bans: " + m_BanApi.GetTotalItemCount() + " | Page: " + (m_BanApi.GetPage() + 1) +"/" + m_BanApi.GetPageCount() + "\n";
		banListStr += "- Identity Id | Banned name\n";
		foreach (OnlineBanListData ban : banList)
		{
			banListStr += string.Format("- %1 | %2\n", ban.identityId, ban.bannedName);
		}
	
		return ScrServerCmdResult(banListStr, EServerCmdResultType.OK);
	}

	// Command handler for RCON and Chat
	//-----------------------------------------------------------------------------
	protected ScrServerCmdResult HandleCommand(array<string> argv, int playerId = 0)
	{	
		if (RplSession.Mode() != RplMode.Dedicated)
			return ScrServerCmdResult("Command is supported only on Dedicated Servers", EServerCmdResultType.ERR);
		
		BackendApi bApi = GetGame().GetBackendApi();
		if (!bApi)
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.ERR);
		m_BanApi = bApi.GetBanServiceApi();
		if (!m_BanApi)
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.ERR);
		
		m_eSubcommandArg = SCR_EBanSubcommandArg.EBSA_MISSING;
		if (argv.Count() > 1)
		{
			if (argv[1] == "create")
				m_eSubcommandArg = SCR_EBanSubcommandArg.EBSA_CREATE;
			else if (argv[1] == "remove")
				m_eSubcommandArg = SCR_EBanSubcommandArg.EBSA_REMOVE;
			else if (argv[1] == "list")
				m_eSubcommandArg = SCR_EBanSubcommandArg.EBSA_LIST;
		}
		 
		
		if (m_eSubcommandArg == SCR_EBanSubcommandArg.EBSA_MISSING)
			m_eSubcommandArg = SCR_EBanSubcommandArg.EBSA_HELP;
		
		m_Callback = new StateBackendCallback;
		
		switch(m_eSubcommandArg)
		{
			// Handle Help
			case SCR_EBanSubcommandArg.EBSA_HELP:
				return ScrServerCmdResult("Help for ban command. \n#ban create <playerId/playerName> <duration> <reason> \n#ban create <identityId/playerName> <duration> <reason> \n#ban remove <identityId/playerName>\n#ban list <page>\n\n- <duration> is in seconds and can be set to 0 for permanent.\n- <reason> is optional.", EServerCmdResultType.OK);
			
			// Handle Create
			case SCR_EBanSubcommandArg.EBSA_CREATE:
				return CreateBan(argv, playerId);
			
			// Handle Remove
			case SCR_EBanSubcommandArg.EBSA_REMOVE:
				return RemoveBan(argv, playerId);
			
			// Handle List
			case SCR_EBanSubcommandArg.EBSA_LIST:
				return ListBans(argv, playerId);
		}
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.ERR);	
	}

	// Specify what kind of player identifier is used
	//-----------------------------------------------------------------------------
	protected SCR_EPlayerIdType GetPlayerIdType(string id)
	{
		if (id.Length() == 36)
		{
			if (id.ContainsAt("-", 8) && id.ContainsAt("-", 13) && id.ContainsAt("-", 18) && id.ContainsAt("-", 23))
				return SCR_EPlayerIdType.EPIT_IDENTITY;
			else
				return SCR_EPlayerIdType.EPIT_USERNAME;
		}
		else if (SCR_StringHelper.IsFormat(SCR_EStringFormat.DIGITS_ONLY, id))
		{
			return SCR_EPlayerIdType.EPIT_PLAYERID;
		}
		else
		{
			return SCR_EPlayerIdType.EPIT_USERNAME;
		}

		return string.Empty;
	}

	// Find playerId from given player name
	//-----------------------------------------------------------------------------
	protected int GetPlayerIdFromName(string name)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);
		foreach (int pid : playerIds)
		{
			if (GetGame().GetPlayerManager().GetPlayerName(pid) == name)
				return pid;
		}

		return 0;
	}

	/* 
	If player name contains spaces, assign the whole string to ID position in argv array
	Player name containing spaces has to be wrapped in quotes (eg. #ban create "Name with spaces" 100)
	*/
	//-----------------------------------------------------------------------------
	protected array<string> TryParsePlayerNameWithSpaces(array<string> argv, int playerNameStartId)
	{
		array<string> result = argv;
		if (!argv[playerNameStartId].ContainsAt("\"", 0))
			return result;

		int lastQuotesIndex = -1;
		for (int i = argv.Count()-1; i > playerNameStartId; --i)
		{
			if (argv[i].IsEmpty())
				continue;

			if (argv[i].ContainsAt("\"", argv[i].Length()-1))
			{
				lastQuotesIndex = i;
				break;
			}
		}

		if (lastQuotesIndex > -1)
		{
			string playerName = argv[playerNameStartId];
			int i;
			for (i = playerNameStartId + 1; i <= lastQuotesIndex; ++i)
			{
				playerName += (" " + argv[i]);
			}

			result[playerNameStartId] = playerName.Substring(1, playerName.Length()-2);
			for (i = lastQuotesIndex; i > playerNameStartId; --i)
			{
				result.Remove(i);
			}
		}

		return result;
	}

	// Specify keyword of command
	//-----------------------------------------------------------------------------
	override string GetKeyword()
	{
		return "ban";
	}
	
	// Run command server-side
	//-----------------------------------------------------------------------------
	override bool IsServerSide()
	{
		return true;
	}
	
	// Set requirement to admin permission via RCON
	//-----------------------------------------------------------------------------
	override int RequiredRCONPermission()
	{
		return ERCONPermissions.PERMISSIONS_ADMIN;
	}
	
	// Set requirement to be logged in administrator for chat command
	//-----------------------------------------------------------------------------
	override int RequiredChatPermission()
	{
		return EPlayerRole.ADMINISTRATOR;
	}
	
	// Handle Chat command on server
	//-----------------------------------------------------------------------------
	override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId)
	{
		return HandleCommand(argv, playerId);
	}
	
	// Handle Chat command on client
	//-----------------------------------------------------------------------------
	override ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId)
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}
	
	// Handle RCON command on server
	//-----------------------------------------------------------------------------
	override ref ScrServerCmdResult OnRCONExecution(array<string> argv)
	{
		return HandleCommand(argv);
	}
	
	// Handle successful result in OnUpdate()
	//-----------------------------------------------------------------------------
	protected ScrServerCmdResult HandleSuccessfulResult()
	{
		switch(m_eSubcommandArg)
		{
			// Handle Create Result
			case SCR_EBanSubcommandArg.EBSA_CREATE:
				if (m_iBanPlayerId > 0)
					GetGame().GetPlayerManager().KickPlayer(m_iBanPlayerId, PlayerManagerKickReason.BAN);
				return ScrServerCmdResult(string.Format("Player '%1' banned!", m_sPlayerName), EServerCmdResultType.OK);
			
			// Handle Remove Result
			case SCR_EBanSubcommandArg.EBSA_REMOVE:
				return ScrServerCmdResult("Ban removed!", EServerCmdResultType.OK);
		
			// Handle List Result
			case SCR_EBanSubcommandArg.EBSA_LIST:
				return ListBansResult();
			
			case SCR_EBanSubcommandArg.EBSA_LIST_FILTERED:
				return ListBansResultFiltered();
		}
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}
	
	// Handle failure in OnUpdate()
	//-----------------------------------------------------------------------------	
	protected ScrServerCmdResult HandleFailure()
	{
		switch (m_Callback.m_eLastError)
		{
			case EApiCode.EACODE_ERROR_MULTIPLE_TARGET_MATCH:
			{
				m_eSubcommandArg = SCR_EBanSubcommandArg.EBSA_LIST_FILTERED;
				m_Callback = new StateBackendCallback;

				m_Params = new BanListPageParams();
				m_Params.playerName = m_sPlayerName;
				m_Params.playerNameMatchType = EStringMatchType.ESMT_STARTS_WITH;
				m_Params.offset = m_Params.limit * (m_iPage - 1);
				
				if (GetPlayerIdFromName(m_sPlayerName) == 0)
					m_Params.limit = BAN_LIST_ITEMS_PER_PAGE_RCON;
				else
					m_Params.limit = BAN_LIST_ITEMS_PER_PAGE_CHAT;

				bool success = m_BanApi.RequestServerBanList(m_Callback, m_Params);
				if (!success)
					return ScrServerCmdResult("Failed to list bans.", EServerCmdResultType.ERR);

				return ScrServerCmdResult(string.Empty, EServerCmdResultType.PENDING);
			} break;

			case EApiCode.EACODE_ERROR_NOT_FOUND:
			{
				if (m_Callback.m_eLastMatchType == EStringMatchType.ESMT_EQUALS)
				{
					m_Callback = new StateBackendCallback;
					m_Callback.m_eLastMatchType = EStringMatchType.ESMT_STARTS_WITH;

					bool success = m_BanApi.RemoveBanByName(m_Callback, m_sPlayerName, EStringMatchType.ESMT_STARTS_WITH);
					PrintFormat("Retrying unban of user with user name: '%1'", m_sPlayerName);
					if (!success)
						return ScrServerCmdResult("Failed to remove the ban.", EServerCmdResultType.ERR);

					return ScrServerCmdResult(string.Empty, EServerCmdResultType.PENDING);					
				}

				return ScrServerCmdResult("User not found!", EServerCmdResultType.ERR);
			} break;

			case EApiCode.EACODE_ERROR_USER_NOT_FOUND:
			{
				return ScrServerCmdResult("User not found!", EServerCmdResultType.ERR);
			} break;
			
			case EApiCode.EACODE_ERROR_UNKNOWN:
			{
				return ScrServerCmdResult("Unknown error.", EServerCmdResultType.ERR);
			} break;
		}

		return ScrServerCmdResult(string.Empty, EServerCmdResultType.ERR);
	}
	
	// Handle Pending command
	//-----------------------------------------------------------------------------
	override ref ScrServerCmdResult OnUpdate()
	{
		switch(m_Callback.m_eState)
		{
			case EBackendCallbackState.EBCS_SUCCESS: return HandleSuccessfulResult();
			case EBackendCallbackState.EBCS_PENDING: return ScrServerCmdResult(string.Empty, EServerCmdResultType.PENDING);
			case EBackendCallbackState.EBCS_TIMEOUT: return ScrServerCmdResult("Timeout", EServerCmdResultType.ERR);
			case EBackendCallbackState.EBCS_ERROR:	 return HandleFailure();
		}
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.ERR);	
	}
}


// Kick Server Command
class KickCommand: ScrServerCommand
{
	// Specify keyword of command
	//-----------------------------------------------------------------------------
	override string GetKeyword()
	{
		return "kick";
	}
	
	// Run command server-side
	//-----------------------------------------------------------------------------
	override bool IsServerSide()
	{
		return true;
	}
	
	// Set requirement to admin permission via RCON
	//-----------------------------------------------------------------------------
	override int RequiredRCONPermission()
	{
		return ERCONPermissions.PERMISSIONS_ADMIN;
	}
	
	// Set requirement to be logged in administrator for chat command
	//-----------------------------------------------------------------------------
	override int RequiredChatPermission()
	{
		return EPlayerRole.ADMINISTRATOR;
	}
	
	// Shared handle for kicking player for RCON and Chat command
	//-----------------------------------------------------------------------------
	protected ScrServerCmdResult KickPlayer(array<string> argv, int playerId = 0)
	{
		if (argv.Count() < 2)
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.PARAMETERS);
		
		int kickPlayerId = argv[1].ToInt();
		if (kickPlayerId < 1)
			return ScrServerCmdResult("Not Valid Player ID", EServerCmdResultType.ERR);
		
		string kickPlayerName = GetGame().GetPlayerManager().GetPlayerName(kickPlayerId);
		if (kickPlayerName == "")
			return ScrServerCmdResult("Player not found", EServerCmdResultType.ERR);
			
		PrintFormat("Kick %1[%2]", kickPlayerName, kickPlayerId);
		GetGame().GetPlayerManager().KickPlayer(kickPlayerId, PlayerManagerKickReason.KICK);
		
		return ScrServerCmdResult(string.Format("Player '%1' kicked!", kickPlayerName), EServerCmdResultType.OK);
	}
	
	// Handle Chat command on server
	//-----------------------------------------------------------------------------
	override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId)
	{
		return KickPlayer(argv, playerId);
	}
	
	// Handle Chat command on client
	//-----------------------------------------------------------------------------
	override ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId)
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}
	
	// Handle RCON command on server
	//-----------------------------------------------------------------------------
	override ref ScrServerCmdResult OnRCONExecution(array<string> argv)
	{
		return KickPlayer(argv);
	}
	
	// Handle Pending command
	//-----------------------------------------------------------------------------
	override ref ScrServerCmdResult OnUpdate()
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}

}