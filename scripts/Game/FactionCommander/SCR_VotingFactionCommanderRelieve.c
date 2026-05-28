[BaseContainerProps(configRoot:true), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingFactionCommanderRelieve : SCR_VotingReferendum
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(int value, bool isOngoing)
	{
		SCR_GameModeCampaign campaignGameMode = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (!campaignGameMode || !campaignGameMode.GetCommanderRoleEnabled())
			return false;

		SCR_FactionManager fManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!fManager)
			return false;

		Faction playerFaction = fManager.GetPlayerFaction(value);
		if (!playerFaction)
			return false;

		SCR_Faction playerFactionLocal = SCR_Faction.Cast(fManager.GetLocalPlayerFaction());
		if (!playerFactionLocal || playerFactionLocal != playerFaction)
			return false;

		return value == playerFactionLocal.GetCommanderId();
	}

	//------------------------------------------------------------------------------------------------
	override int GetPlayerCount()
	{
		int playerCount;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());

		Faction targetFaction;

		if (factionManager)
			targetFaction = factionManager.GetPlayerFaction(m_iValue);

		//--- Count only players on the same faction as target of the vote
		//--- e.g., with 50% vote limit, only half of BLUFOR players will have to vote, not half of all players

		Faction playerFaction;
		array<int> players = {};

		for (int i = 0, count = GetGame().GetPlayerManager().GetPlayers(players); i < count; i++)
		{
			playerFaction = factionManager.GetPlayerFaction(players[i]);

			if (targetFaction == playerFaction)
				playerCount++;
		}

		//--- Limit to prevent instant completion in a session with less than limited participants
		return Math.Max(playerCount, m_iMinVotes);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanSendNotification(int value)
	{
		SCR_FactionManager fManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!fManager)
			return false;

		return fManager.GetPlayerFaction(value) == fManager.GetLocalPlayerFaction();
	}

	//------------------------------------------------------------------------------------------------
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		if (winner == DEFAULT_VALUE)
			return;

		if (Replication.IsClient())
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(winner));
		if (!faction || faction.GetCommanderId() != winner)
			return;

		SCR_FactionCommanderHandlerComponent component = SCR_FactionCommanderHandlerComponent.GetInstance();
		if (!component)
			return;

		component.SetFactionCommander(SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(winner)), SCR_Faction.AI_COMMANDER_ID);
	}
}