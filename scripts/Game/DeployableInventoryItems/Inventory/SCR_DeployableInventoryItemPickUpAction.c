class SCR_DeployableInventoryItemPickUpAction : SCR_PickUpItemAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;
		
		SCR_BaseDeployableInventoryItemComponent deployableItemComponent = SCR_BaseDeployableInventoryItemComponent.Cast(owner.FindComponent(SCR_BaseDeployableInventoryItemComponent));
		if (!deployableItemComponent)
			return true;
		
		if (deployableItemComponent.IsDeployed())
			return false;
		
		return super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;
		
		SCR_BaseDeployableInventoryItemComponent deployableItemComponent = SCR_BaseDeployableInventoryItemComponent.Cast(owner.FindComponent(SCR_BaseDeployableInventoryItemComponent));
		if (!deployableItemComponent)
			return true;
		
		if (deployableItemComponent.IsDeploying())
			return false;
		
		return super.CanBePerformedScript(user);
	}
}