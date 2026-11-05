class SCR_WheelSlotInfo : WheelSlotInfo
{
	//------------------------------------------------------------------------------------------------
	override void OnAttachedEntity(IEntity entity)
	{
		HitZoneContainerComponent hitzoneContainer = HitZoneContainerComponent.Cast(entity.FindComponent(HitZoneContainerComponent));
		if (!hitzoneContainer)
			return;
		
		SCR_WheelHitZone hitZone = SCR_WheelHitZone.Cast(hitzoneContainer.GetDefaultHitZone());
		if (hitZone)
			hitZone.SetWheelIndex(GetWheelIndex());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDetachedEntity(IEntity entity)
	{
		HitZoneContainerComponent hitzoneContainer = HitZoneContainerComponent.Cast(entity.FindComponent(HitZoneContainerComponent));
		if (!hitzoneContainer)
			return;
		
		SCR_WheelHitZone hitZone = SCR_WheelHitZone.Cast(hitzoneContainer.GetDefaultHitZone());
		if (hitZone)
			hitZone.SetWheelIndex(-1);
	}
};
