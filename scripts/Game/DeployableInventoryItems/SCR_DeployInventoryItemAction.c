class SCR_DeployInventoryItemAction : SCR_DeployInventoryItemBaseAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (GetOwner().GetParent())
			return false;
		
		return super.CanBeShownScript(user);
	}
}
