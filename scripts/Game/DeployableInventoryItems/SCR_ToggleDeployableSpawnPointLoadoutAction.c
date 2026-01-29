class SCR_ToggleDeployableSpawnPointLoadoutAction : ScriptedUserAction
{
	protected SCR_RestrictedDeployableSpawnPointComponent m_DeployableItemComp;
	
	//------------------------------------------------------------------------------------------------
	override protected bool CanBeShownScript(IEntity user)
	{
		return m_DeployableItemComp.CanToggleLoadoutSpawningBeShown(user);
	}	
	
	//------------------------------------------------------------------------------------------------
	override protected void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_DeployableItemComp.ToggleSpawningWithLoadout(pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{	
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		m_DeployableItemComp = SCR_RestrictedDeployableSpawnPointComponent.Cast(owner.FindComponent(SCR_RestrictedDeployableSpawnPointComponent));
	}
}