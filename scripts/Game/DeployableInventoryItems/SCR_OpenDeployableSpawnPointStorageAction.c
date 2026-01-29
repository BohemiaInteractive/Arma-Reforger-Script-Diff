class SCR_OpenDeployableSpawnPointStorageAction : SCR_OpenStorageAction
{
#ifndef DISABLE_INVENTORY
	protected SCR_RestrictedDeployableSpawnPointComponent m_DeployableItemComp;
	
	//------------------------------------------------------------------------------------------------
	override protected bool CanBeShownScript(IEntity user)
	{
		return m_DeployableItemComp.CanOpenStorageBeShown(user);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{	
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		m_DeployableItemComp = SCR_RestrictedDeployableSpawnPointComponent.Cast(owner.FindComponent(SCR_RestrictedDeployableSpawnPointComponent));
	}
#endif	
}