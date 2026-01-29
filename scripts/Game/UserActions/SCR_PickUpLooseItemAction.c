class SCR_PickUpLooseItemAction : SCR_PickUpItemAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return !GetOwner().GetParent() && super.CanBeShownScript(user);
	}
}