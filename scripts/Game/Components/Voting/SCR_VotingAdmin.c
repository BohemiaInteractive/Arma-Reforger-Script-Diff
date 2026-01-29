[BaseContainerProps(configRoot:true), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingAdmin : SCR_VotingReferendum
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(int value, bool isOngoing)
	{
		//--- Cannot vote admin when the session is hosted
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		return votingManager && votingManager.GetHostPlayerID() == 0;
	}

	//------------------------------------------------------------------------------------------------
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		if (winner != DEFAULT_VALUE)
			GetGame().GetPlayerManager().GivePlayerRole(winner, EPlayerRole.SESSION_ADMINISTRATOR);
	}
}
