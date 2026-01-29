[EntityEditorProps(category: "GameScripted/FiringRange", description: "Entity handling communication player to server.")]
class SCR_FiringRangeNetworkEntityClass : GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_FiringRangeNetworkEntity : GenericEntity
{

	protected static SCR_FiringRangeNetworkEntity s_CommEnt;
	
	//------------------------------------------------------------------------------------------------
	static SCR_FiringRangeNetworkEntity GetInstance()
	{
		return s_CommEnt;
	}
	
	//------------------------------------------------------------------------------------------------
	void CalculateTargetDistance(notnull IEntity pOwnerEntity, bool increase)
	{	
		Rpc(RpcAsk_CalculateTargetDistance, Replication.FindId(pOwnerEntity), increase);
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearPlayerScore(int playerID, SCR_FiringRangeManager firingRangeManager)
	{
		Rpc(RpcAsk_ClearPlayerScore, playerID, Replication.FindId(firingRangeManager));
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerScoreMax(int playerID, int maxScore, SCR_FiringRangeManager firingRangeManager)
	
	{
		Rpc(RpcAsk_SetMaxLineScore, playerID, maxScore, Replication.FindId(firingRangeManager));
	}
			
	//------------------------------------------------------------------------------------------------
	void ErectRandomTargets(notnull IEntity pOwnerEntity, int playerID)
	{
		Rpc(RpcAsk_ErectRandomTargets, Replication.FindId(pOwnerEntity), playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void BackToDefaultTarget(notnull IEntity pOwnerEntity)
	{
		Rpc(RpcAsk_BackToDefaultTarget, Replication.FindId(pOwnerEntity));
	}
		
	//------------------------------------------------------------------------------------------------
	void RemoveIndicators(IEntity pOwnerEntity, SCR_FiringRangeManager firingRangeManager)
	{
		if (!pOwnerEntity)
			return;

		Rpc(RpcAsk_RemoveIndicatorsServer, Replication.FindId(pOwnerEntity), Replication.FindId(firingRangeManager));		
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterCommEntity(RplId comEntID)
	{
		Rpc(RpcAsk_RegisterCommEntity, comEntID);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddTarget(notnull IEntity pOwnerEntity, bool increase)
	{	
		Rpc(RpcAsk_AddTarget, Replication.FindId(pOwnerEntity), increase);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcAsk_RegisterCommEntity(RplId comEntID)
	{
		IEntity ent = IEntity.Cast(Replication.FindItem(comEntID));
		if (!ent)
			return;
		
		s_CommEnt = SCR_FiringRangeNetworkEntity.Cast(ent);
	}
			
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemoveIndicatorsServer(RplId controllerReplicationId, RplId firingRangeManager)
	{
		SCR_FiringRangeManager manager = SCR_FiringRangeManager.Cast(Replication.FindItem(firingRangeManager));
		if (manager)
			manager.RemoveIndicators(controllerReplicationId);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CalculateTargetDistance(RplId controllerReplicationId, bool increase)
	{
		SCR_FiringRangeController lineController = SCR_FiringRangeController.Cast(Replication.FindItem(controllerReplicationId));
		if (lineController)
			lineController.CalculateTargetDistance(increase);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ClearPlayerScore(int playerID, RplId firingRangeManager)
	{		
		SCR_FiringRangeManager manager = SCR_FiringRangeManager.Cast(Replication.FindItem(firingRangeManager));
		if (manager)
			manager.ClearPlayerScore(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ErectRandomTargets(RplId targetReplicationId, int playerID)
	{
		IEntity pOwnerEntity = IEntity.Cast(Replication.FindItem(targetReplicationId));
		SCR_FiringRangeController lineController = SCR_FiringRangeController.Cast(pOwnerEntity);
		if (lineController)
			lineController.AnimateTargets(pOwnerEntity,playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_BackToDefaultTarget(RplId targetReplicationId)
	{
		SCR_FiringRangeController lineController = SCR_FiringRangeController.Cast(Replication.FindItem(targetReplicationId));
		if (lineController)
			lineController.BackToDefaultTarget();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetMaxLineScore(int playerID, int maxScore, RplId firingRangeManager)
	{
		SCR_FiringRangeManager manager = SCR_FiringRangeManager.Cast(Replication.FindItem(firingRangeManager));
		if (manager)
			manager.SetPlayerScoreMax(playerID, maxScore);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CountPlayerScore(int playerID, int hitValue, RplId firingRangeManager)
	{
		SCR_FiringRangeManager manager = SCR_FiringRangeManager.Cast(Replication.FindItem(firingRangeManager));
		if (manager)
			manager.CountPlayerScore(playerID, hitValue);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AddTarget(RplId controllerReplicationId, bool increase)
	{
		SCR_FiringRangeController lineController = SCR_FiringRangeController.Cast(Replication.FindItem(controllerReplicationId));
		if (lineController)
			lineController.UpdateNumberOfTargets(increase);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_FiringRangeNetworkEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
	}
};
