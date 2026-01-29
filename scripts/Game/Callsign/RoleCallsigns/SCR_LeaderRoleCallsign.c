[BaseContainerProps(), BaseContainerCustomStringTitleField("Leader")]
class SCR_LeaderRoleCallsign: SCR_BaseRoleCallsign
{
	//------------------------------------------------------------------------------------------------
	override bool IsValidRole(IEntity character, int playerID, SCR_AIGroup group, inout int roleCallsignIndex, out bool isUnique)
	{
		if (!character)
			return false;
		
		//~ Slave group can never have the leader role
		if (!group || group.IsSlave())
			return false;
		
		if (!super.IsValidRole(character, playerID, group, roleCallsignIndex, isUnique))
			return false;
		
		if (group.GetLeaderEntity() == character)
		{
			roleCallsignIndex = GetRoleIndex();
			return true;
		}
	
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetRoleIndex()
	{
		return ERoleCallsign.SQUAD_LEADER;
	}
};