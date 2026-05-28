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