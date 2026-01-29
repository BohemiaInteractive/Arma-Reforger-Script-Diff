#define ENABLE_BASE_DESTRUCTION

[EntityEditorProps(category: "GameScripted/Destruction", description: "")]
class SCR_DestructionSynchronizationComponentClass : ScriptComponentClass
{
}

class SCR_DestructionSynchronizationComponent : ScriptComponent
{
#ifdef ENABLE_BASE_DESTRUCTION
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rplId
	//! \param[in] index
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_RequestDestructibleState(RplId rplId, int index)
	{
		SCR_MPDestructionManager destructionManager = SCR_MPDestructionManager.GetInstance();
		if (!destructionManager)
			return;
		
		SCR_DestructionDamageManagerComponent destructible = destructionManager.FindDynamicallySpawnedDestructibleByIndex(rplId, index);
		if (!destructible)
			return;
		
		destructible.ReplicateDestructibleState();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rplId
	//! \param[in] index
	void RequestDestructibleState(RplId rplId, int index)
	{
		Rpc(RPC_RequestDestructibleState, rplId, index);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_DestructionSynchronizationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}
#endif
}
