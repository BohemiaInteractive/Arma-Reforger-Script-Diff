class SCR_BatteryHitZone: SCR_VehicleHitZone
{
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: Collider ID retrieval is obsolete
	//------------------------------------------------------------------------------------------------
	//! Called when hit zone is initialized
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		array<string> colliderNames = {};
		GetAllColliderNames(colliderNames);
		if (colliderNames.IsEmpty())
			return;
		
		Physics physics = pOwnerEntity.GetPhysics();
		if (!physics)
			return;
		
		SCR_PowerComponent powerComp = SCR_PowerComponent.Cast(pOwnerEntity.FindComponent(SCR_PowerComponent));
		if (!powerComp)
			return;
		
		// Only register battery hitzones that have colliders. 
		// Logical hitzones are not supported.
		foreach (string colliderName: colliderNames)
		{
			if (physics.GetGeom(colliderName) == -1)
				continue;
			
			powerComp.RegisterBatteryHitZone(this);
			return;
		}
	}
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when the damage state changes.
	*/
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		SCR_PowerComponent powerComp = SCR_PowerComponent.Cast(GetOwner().FindComponent(SCR_PowerComponent));
		if (powerComp)
			powerComp.UpdatePowerState();
	}
};
