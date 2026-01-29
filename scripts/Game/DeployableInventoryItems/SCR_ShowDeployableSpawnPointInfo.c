class SCR_ShowDeployableSpawnPointInfo : ScriptedUserAction
{
	protected SCR_RestrictedDeployableSpawnPointComponent m_DeployableSpawnPointComp;
	
	protected RplComponent m_RplComp;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_DeployableSpawnPointComp)
			return false;
		
		return m_DeployableSpawnPointComp.CanInfoBeShown(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		return true;
 	}
	
 	//------------------------------------------------------------------------------------------------
 	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		if (!m_DeployableSpawnPointComp || !m_RplComp || m_RplComp.IsProxy())
			return;
		
		m_DeployableSpawnPointComp.ShowInfo(pUserEntity);
 	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		m_DeployableSpawnPointComp = SCR_RestrictedDeployableSpawnPointComponent.Cast(owner.FindComponent(SCR_RestrictedDeployableSpawnPointComponent));
		m_RplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
	}
}