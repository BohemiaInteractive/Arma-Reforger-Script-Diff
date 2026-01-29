class SCR_HeadgearPhysicsObserverAttribute : SCR_PhysicsObserverAttribute
{
	//------------------------------------------------------------------------------------------------
	override void OnPhysicsStateChanged(notnull IEntity owner, bool isActive)
	{
		SCR_HeadgearInventoryItemComponent iic = SCR_HeadgearInventoryItemComponent.Cast(owner.FindComponent(SCR_HeadgearInventoryItemComponent));
		if (iic)
			iic.OnPhysicsStateChanged(isActive);
	}
}
