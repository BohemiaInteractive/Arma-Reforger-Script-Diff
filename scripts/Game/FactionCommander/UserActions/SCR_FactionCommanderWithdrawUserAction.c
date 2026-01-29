class SCR_FactionCommanderWithdrawUserAction : SCR_FactionCommanderVolunteerUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_VotingManager || !m_SignupComponent)
			return false;

		SCR_Faction playerFaction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (!playerFaction)
			return false;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);

		if (playerFaction.GetCommanderId() != playerId)
			return false;

		return m_SignupComponent.IsControlledByFaction(playerFaction);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_VotingManager)
			return;

		PlayerController pc = GetGame().GetPlayerController();

		if (!pc)
			return;

		SCR_VoterComponent comp = SCR_VoterComponent.Cast(pc.FindComponent(SCR_VoterComponent));

		if (!comp)
			return;

		comp.Vote(EVotingType.COMMANDER_WITHDRAW, pc.GetPlayerId());
	}
}
