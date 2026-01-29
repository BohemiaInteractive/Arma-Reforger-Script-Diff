//------------------------------------------------------------------------------------------------
class SCR_SetTargetDistanceUpUserAction : ScriptedUserAction
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
		firingRangeNetworkEntity.CalculateTargetDistance(pOwnerEntity, true);
	}
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		// Check if the firing line is currently assigned to any player. If so, other players can't interact with this firing range controller.
		return m_FiringRangeManager && m_LineController.GetFiringLineOwnerId() == m_LineController.NO_TARGET_OWNER;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return m_LineController && !m_LineController.IsHighestDistanceSet();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = ("#AR-FiringRange_ActionDistanceUp-UC");
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

};
