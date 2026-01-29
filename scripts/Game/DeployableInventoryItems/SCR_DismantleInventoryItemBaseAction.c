class SCR_DismantleInventoryItemBaseAction : ScriptedUserAction
{
	protected SCR_BaseDeployableInventoryItemComponent m_DeployableItemComp;
	
	protected RplComponent m_RplComp;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_DeployableItemComp)
			return false;
		
		return m_DeployableItemComp.CanDismantleBeShown(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		return true;
 	}
	
 	//------------------------------------------------------------------------------------------------
 	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		if (!m_DeployableItemComp || !m_RplComp || m_RplComp.IsProxy())
			return;
		
		m_DeployableItemComp.Dismantle(pUserEntity);
 	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_DeployableItemComp = SCR_BaseDeployableInventoryItemComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseDeployableInventoryItemComponent));
		m_RplComp = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
	}
}