//------------------------------------------------------------------------------------------------
class SCR_SetTargetsModeUserAction : ScriptedUserAction
{
	// Member variables
	private SCR_FiringRangeController m_LineController;
	private SCR_FiringRangeManager m_FiringRangeManager;
	private SignalsManagerComponent m_SignalManager;
	private int m_iPushButtonStart;
	
	const static int RESET_BUTTON_TIME = 500;
	const static float BUTTON_PRESSED_STATE = -0.005;
	const static int BUTTON_DEFAULT_STATE = 0;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) 
	{
		m_LineController = SCR_FiringRangeController.Cast(pOwnerEntity);
		m_FiringRangeManager = SCR_FiringRangeManager.Cast(pOwnerEntity.GetParent());
		m_SignalManager = SignalsManagerComponent.Cast(pOwnerEntity.FindComponent(SignalsManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		// Find conflict network entity to send RPC to server		
		SCR_FiringRangeNetworkEntity firingRangeNetworkEntity = SCR_FiringRangeNetworkEntity.GetInstance();
		if (!firingRangeNetworkEntity)
			return;
		
		// Get player ID
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		
		// Clear player score from previous match
		firingRangeNetworkEntity.ClearPlayerScore(playerID, m_FiringRangeManager);
		// Delete all decals on indicator of the given firing line
		firingRangeNetworkEntity.RemoveIndicators(pOwnerEntity, m_FiringRangeManager);
		// Count maximal possible score player can foreach
		firingRangeNetworkEntity.SetPlayerScoreMax(playerID, m_LineController.GetMaxScoreInRound(), m_FiringRangeManager);
		// Start with the erectign random targets
		firingRangeNetworkEntity.ErectRandomTargets(pOwnerEntity,playerID);
		
		m_iPushButtonStart = m_SignalManager.FindSignal("StartRound");
		m_SignalManager.SetSignalValue(m_iPushButtonStart, BUTTON_PRESSED_STATE);
		GetGame().GetCallqueue().CallLater(ResetStartButton, RESET_BUTTON_TIME, false); 
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
	override bool GetActionNameScript(out string outName)
	{
		outName = ("#AR-FiringRange_ActionStartRound-UC"); 
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetStartButton()
	{
		m_SignalManager.SetSignalValue(m_iPushButtonStart, BUTTON_DEFAULT_STATE);
	}
};