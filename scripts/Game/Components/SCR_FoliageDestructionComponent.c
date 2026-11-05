[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "Component foliage destruction.", color: "0 0 255 255")]
class SCR_FoliageDestructionComponentClass : ScriptComponentClass
{
}

class SCR_FoliageDestructionComponent : ScriptComponent
{
	protected SCR_FoliageDestructionSystem m_FoliageDestructionSystem;

	protected static const float DESTRUCTION_MAXIMUM_DISTANCE = 8;
	protected bool m_bIsFoliageLocked;
	protected EntityID m_ServerAssignedFoliageID;

	//------------------------------------------------------------------------------------------------
	protected bool FetchFoliageSystem()
	{
		World world = GetGame().GetWorld();
		m_FoliageDestructionSystem = SCR_FoliageDestructionSystem.Cast(world.FindSystem(SCR_FoliageDestructionSystem));
		
		return m_FoliageDestructionSystem;
	}

	//------------------------------------------------------------------------------------------------
	//! If true, the server has the correct bush assigned and it is not desynced!
	//! \return
	bool VerifyFoliageSync(IEntity localAssignedFoliage)	
	{
		return localAssignedFoliage && localAssignedFoliage.GetID() == m_ServerAssignedFoliageID;
	}

	//------------------------------------------------------------------------------------------------
	//! If true, the player is currently interacting with a piece of foliage so shouldnt be switching between different ones
	//! \return
	bool GetIsFoliageLocked()	
	{
		return m_bIsFoliageLocked;
	}

	//------------------------------------------------------------------------------------------------
	//! Set the current lock state so the player cant switch between pieces of foliage mid animation
	//! \param[in] isLocked
	void SetIsFoliageLocked(bool isLocked)	
	{
		m_bIsFoliageLocked = isLocked;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign the foliage for this players foliage destruction helper
	//! \param[in] treeID Entity ID number for 
	void RequestSetHelperTree(EntityID treeID)
	{
		if (!m_bIsFoliageLocked)
			Rpc(RpcAsk_SetHelperTree, treeID);
	}

	//------------------------------------------------------------------------------------------------
	//! Request serverside verification for targeted tree for destruction helper
	//! \param[in] treeID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetHelperTree(EntityID treeID)
	{
		PlayerController sender = PlayerController.Cast(GetOwner());
		if (!sender)
			return;

		IEntity player = sender.GetControlledEntity();
		IEntity treeEnt = GetOwner().GetWorld().FindEntityByID(treeID);
		Tree tree = Tree.Cast(treeEnt);
		if (!tree)
			return;

		if (!m_FoliageDestructionSystem && !FetchFoliageSystem())
			return;

		IEntity foliageHelper = m_FoliageDestructionSystem.GetHelper(sender.GetPlayerId());		

		// distance check to ensure people arent uprooting trees 2 km away
		if (vector.DistanceSq(player.GetOrigin(), tree.GetOrigin()) > DESTRUCTION_MAXIMUM_DISTANCE * DESTRUCTION_MAXIMUM_DISTANCE)
			return;	

		// ensure that this tree is destructible by player manually
		TreeClass treeClass = TreeClass.Cast(tree.GetPrefabData());
		if (!treeClass || !treeClass.m_bDestructibleByHand)
			return;

		vector position = tree.GetOrigin();
		position[1] = position[1] + treeClass.m_fDestroyActionHeight;

		foliageHelper.SetOrigin(position);
		foliageHelper.Update();

		RplComponent rplComp = RplComponent.Cast(foliageHelper.FindComponent(RplComponent));
		if (rplComp)
			Rpc(RpcDo_SetHelperPosition, rplComp.Id(), position, treeID);

		RpcDo_SetHelperPosition(rplComp.Id(), position, treeID);
	}

	//------------------------------------------------------------------------------------------------
	//! Replicates the new position and assigned foliage of the tree destruction helper
	//! \param[in] rplID replication ID of the tree destruction helper
	//! \param[in] actionPosition target position of the helper.
	//! \param[in] treeID
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SetHelperPosition(RplId rplID, vector actionPosition, EntityID treeID)
	{
		m_ServerAssignedFoliageID = treeID;

		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplID));
		if (!rplComp)
			return;

		IEntity helperEnt = rplComp.GetEntity();

		IEntity treeEnt = GetOwner().GetWorld().FindEntityByID(treeID);
		if (!treeEnt)
			return;

		Tree tree = Tree.Cast(treeEnt);

		helperEnt.SetOrigin(actionPosition);
		helperEnt.Update();

		ActionsManagerComponent actionMgr = ActionsManagerComponent.Cast(helperEnt.FindComponent(ActionsManagerComponent));
		if (!actionMgr)
			return;

		SCR_CutFoliageAction cut = SCR_CutFoliageAction.Cast(actionMgr.GetFirstAction());
		if (cut)
			cut.AssignTargetFoliage(tree);
	}
}
