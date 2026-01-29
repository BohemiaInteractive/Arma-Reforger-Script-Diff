[BaseContainerProps(configRoot:true), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingFactionCommander : SCR_VotingReferendum
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

		if (!playerFactionLocal)
			return false;

		if (isOngoing)
			return (playerFaction == playerFactionLocal);

		PlayerController playerController = GetGame().GetPlayerController();

		if (!playerController)
			return false;

		if (value != playerController.GetPlayerId())
			return false;

		if (value == playerFactionLocal.GetCommanderId())
			return false;

		SCR_FactionCommanderHandlerComponent handlerComponent = SCR_FactionCommanderHandlerComponent.GetInstance();

		if (!handlerComponent)
			return false;

		if (handlerComponent.CheckRank())
		{
			IEntity player = playerController.GetControlledEntity();

			if (!player)
				return false;

			if (SCR_CharacterRankComponent.GetCharacterRank(player) < handlerComponent.GetMinimumRank())
				return false;
		}

		SCR_FactionCommanderPlayerComponent comp = SCR_FactionCommanderPlayerComponent.Cast(playerController.FindComponent(SCR_FactionCommanderPlayerComponent));

		if (!comp)
			return false;

		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return false;

		WorldTimestamp currentTimestamp = world.GetServerTimestamp();
		WorldTimestamp ReplaceCommanderCooldown = comp.GetReplaceCommanderCooldownTimestamp();
		if (ReplaceCommanderCooldown.Greater(currentTimestamp))
			return false;

		return world.GetServerTimestamp().GreaterEqual(comp.GetNextVolunteeringTimestamp());
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

		return (fManager.GetPlayerFaction(value) == fManager.GetLocalPlayerFaction());
	}

	//------------------------------------------------------------------------------------------------
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		if (winner == DEFAULT_VALUE)
			return;

		if (Replication.IsClient())
			return;

		SCR_FactionCommanderHandlerComponent component = SCR_FactionCommanderHandlerComponent.GetInstance();

		if (!component)
			return;

		component.SetFactionCommander(SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(winner)), winner);
	}

	//------------------------------------------------------------------------------------------------
	override int GetWinner()
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return 0;

		SCR_Faction targetFaction = SCR_Faction.Cast(factionManager.GetPlayerFaction(m_iValue));
		if (!targetFaction)
			return 0;

		// Current Commander is AI, appoint first volunteer as Commander
		if (targetFaction.GetCommanderId() <= 0)
			return m_iValue;

		return super.GetWinner();
	}
}

[BaseContainerProps(configRoot:true), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingFactionCommanderWithdraw : SCR_VotingReferendum
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(int value, bool isOngoing)
	{
		SCR_Faction playerFactionLocal = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (!playerFactionLocal)
			return false;

		int playerIdLocal = GetGame().GetPlayerController().GetPlayerId();

		return (value == playerIdLocal && playerFactionLocal.GetCommanderId() == playerIdLocal);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanSendNotification(int value)
	{
		SCR_FactionManager fManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());

		if (!fManager)
			return false;

		return (fManager.GetPlayerFaction(value) == fManager.GetLocalPlayerFaction());
	}

	//------------------------------------------------------------------------------------------------
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		if (winner == DEFAULT_VALUE)
			return;

		if (Replication.IsClient())
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(winner));

		if (!faction)
			return;

		if (faction.GetCommanderId() != winner)
			return;

		SCR_FactionCommanderHandlerComponent component = SCR_FactionCommanderHandlerComponent.GetInstance();

		if (!component)
			return;

		component.SetFactionCommander(SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(winner)), 0);
	}
}
