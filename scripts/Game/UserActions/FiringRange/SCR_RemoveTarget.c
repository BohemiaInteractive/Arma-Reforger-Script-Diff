//------------------------------------------------------------------------------------------------

class SCR_RemoveTarget : ScriptedUserAction
{
	private SCR_FiringRangeController m_LineController;
	private SCR_FiringRangeManager m_FiringRangeManager;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) 
	{
		m_LineController = SCR_FiringRangeController.Cast(pOwnerEntity);
		m_FiringRangeManager = SCR_FiringRangeManager.Cast(pOwnerEntity.GetParent());
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		// Find firing range network entity to send RPC to server
		SCR_FiringRangeNetworkEntity firingRangeNetworkEntity = SCR_FiringRangeNetworkEntity.GetInstance();
		if (!firingRangeNetworkEntity)
			return;
		
		// Select targets distance
		firingRangeNetworkEntity.AddTarget(pOwnerEntity, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		// Check if the firing line is currently assigned to any player. If so, other players can't interact with this firing range controller.
 		if (m_FiringRangeManager)
			return m_LineController.GetFiringLineOwnerId() == m_LineController.NO_TARGET_OWNER;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		// Don't allow player to set less then 1 target in round
		if (m_LineController.GetTargetsInRound() == 1)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = ("#AR-FiringRange_ActionRemoveTarget-UC");
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	//------------------------------------------------------------------------------------------------
	void SCR_RemoveTarget()
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_RemoveTarget()
	{
	}

};
