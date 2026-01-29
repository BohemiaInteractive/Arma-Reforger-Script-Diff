class SCR_DeployInventoryItemBaseAction : ScriptedUserAction
{
	protected SCR_BaseDeployableInventoryItemComponent m_DeployableItemComp;	
	protected bool m_bActionStarted;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_DeployableItemComp)
			return false;
		
		return m_DeployableItemComp.CanDeployBeShown(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_DeployableItemComp)
			return false;
		
		return m_bActionStarted || !m_DeployableItemComp.IsDeploying();
	}
	
 	//------------------------------------------------------------------------------------------------
 	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		if (!m_DeployableItemComp)
			return;
		
		m_DeployableItemComp.Deploy(pUserEntity);
		GetGame().GetCallqueue().CallLater(ResetDeployingDelayed, 100, param1: pUserEntity); //reset bool later so there is enough time for user action to disappear
 	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetDeployingDelayed(IEntity pUserEntity)
	{
		if (m_DeployableItemComp)
			m_DeployableItemComp.SetDeploying(false);
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (!controlledEntity)
			return;
		
		if (pUserEntity == controlledEntity)
			m_bActionStarted = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		if (!m_DeployableItemComp)
			return;
		
		m_DeployableItemComp.SetDeploying(true);
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (!controlledEntity)
			return;
		
		if (pUserEntity == controlledEntity)
			m_bActionStarted = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_DeployableItemComp)
			return;
		
		m_DeployableItemComp.SetDeploying(false);
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (!controlledEntity)
			return;
		
		if (pUserEntity == controlledEntity)
			m_bActionStarted = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{	
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		m_DeployableItemComp = SCR_BaseDeployableInventoryItemComponent.Cast(owner.FindComponent(SCR_BaseDeployableInventoryItemComponent));
	}
}