class SCR_WheelSlotInfo : RegisteringComponentSlotInfo
{
	[Attribute("-1", UIWidgets.Slider, "VehicleWheeledSimulation wheel index", "-1 1000 1")]
	protected int m_iWheelIndex;
	
	//------------------------------------------------------------------------------------------------
	override void OnAttachedEntity(IEntity entity)
	{
		HitZoneContainerComponent hitzoneContainer = HitZoneContainerComponent.Cast(entity.FindComponent(HitZoneContainerComponent));
		if (!hitzoneContainer)
			return;
		
		SCR_WheelHitZone hitZone = SCR_WheelHitZone.Cast(hitzoneContainer.GetDefaultHitZone());
		if (hitZone)
			hitZone.SetWheelIndex(m_iWheelIndex);
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
