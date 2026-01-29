class SCR_PlayerIdentityUtils
{
	//------------------------------------------------------------------------------------------------
	//! Gets the Bohemia Player Identity ID with polyfill support for PeerTool plugin testing
	//! \param playerId Index of the player inside player manager
	//! \return the uid as UUID wrapper
	static UUID GetPlayerIdentityId(int playerId)
	{
		if (playerId == 0)
			return UUID.NULL_UUID;

		if (!Replication.IsServer())
		{
			#ifdef ENABLE_DIAG
			Debug.Error("GetPlayerIdentityId can only be used on the server and after OnPlayerAuditSuccess.");
			#endif
			return UUID.NULL_UUID;
		}

		string uid = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
		if (!uid)
		{
			if (RplSession.Mode() != RplMode.Dedicated)
			{
				// Peer tool support
				const string playerName = GetGame().GetPlayerManager().GetPlayerName(playerId);

				int splitLength = Math.Max(1, playerName.Length() / 3);
				const string split1 = Math.AbsInt(playerName.Substring(0, splitLength).Hash()).ToString(8, true);
				const string split2 = Math.AbsInt(playerName.Substring(splitLength, splitLength).Hash()).ToString(8, true);
				const int doubleSplit = splitLength * 2;
				const string split3 = Math.AbsInt(playerName.Substring(doubleSplit, playerName.Length() - doubleSplit).Hash()).ToString(8, true);

				// 00bbbddd-SPL1-SPL1-SPL2-SPL2 SPL3 SPL3
				uid = string.Format("00bbbddd-%1-%2-%3-%4%5", split1.Substring(0, 4), split1.Substring(4, 4), split2.Substring(0, 4), split2.Substring(4, 4), split3);
			}
			#ifdef ENABLE_DIAG
			else
			{
				Debug.Error("Dedicated server is not correctly configured to connect to the BI backend.\nSee https://community.bistudio.com/wiki/Arma_Reforger:Server_Config#publicAddress");
			}
			#endif
		}

		return uid;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the Bohemia Player Identity ID
	//! \param player Instance of the player
	//! \return the uid as string
	static string GetPlayerIdentityId(IEntity player)
	{
		return GetPlayerIdentityId(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(player));
	}
}
