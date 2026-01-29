
[BaseContainerProps(), BaseContainerCustomStringTitleField("Player Leader")]
class SCR_PlayerLeaderRoleCallsign: SCR_BaseRoleCallsign
{
	//------------------------------------------------------------------------------------------------
	override bool IsValidRole(IEntity character, int playerID, SCR_AIGroup group, inout int roleCallsignIndex, out bool isUnique)
	{
		//~ Slave group can never have the leader role
		if (!group || group.IsSlave())
			return false;
		
		if (!super.IsValidRole(character, playerID, group, roleCallsignIndex, isUnique))
			return false;
		
		if (group.GetLeaderID() == playerID)
		{
			roleCallsignIndex = GetRoleIndex();
			return true;
		}
	
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetRoleIndex()
	{
		return ERoleCallsign.PLAYER_GROUP_LEADER;
	}
}
