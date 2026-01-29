//~ A kick voting that can only be initiated by system never started manualing by player
[BaseContainerProps(configRoot:true), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingAutoKick : SCR_VotingKick
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(int value, bool isOngoing)
	{
		//~ Voting is only available when ongoing as only the system can start the voting process. Not players manually
		if (!isOngoing)
			return false;

		return super.IsAvailable(value, isOngoing);
	}
}
