class SCR_AIDecoTestIsSlaveGroupOfEntity : DecoratorTestScripted
{	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!controlled)
			return false;
		SCR_AIGroup groupSlave = SCR_AIGroup.Cast(agent);
		if (!groupSlave)
			return false;
		SCR_AIGroup groupMaster = groupSlave.GetMaster();
		if (!groupMaster)
			return false;
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(controlled);
		array<int> playerIDs = {};
		playerIDs = groupMaster.GetPlayerIDs();
		return playerIDs.Contains(playerID);
	}
};