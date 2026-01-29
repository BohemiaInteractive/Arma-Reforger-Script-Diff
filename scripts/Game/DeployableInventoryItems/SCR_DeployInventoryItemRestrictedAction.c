class SCR_DeployInventoryItemRestrictedAction : SCR_DeployInventoryItemBaseAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		SCR_RestrictedDeployableSpawnPointComponent restrictedDeployableSpawnPointComp = SCR_RestrictedDeployableSpawnPointComponent.Cast(m_DeployableItemComp);
		if (!restrictedDeployableSpawnPointComp)
			return false;
		
		string reason;
		bool canBePerformed = restrictedDeployableSpawnPointComp.CanDeployBePerformed(reason);
		if (!reason.IsEmpty())
			SetCannotPerformReason(reason);
		
		if (!canBePerformed)
			return false;
		
		return super.CanBePerformedScript(user);
 	}
}