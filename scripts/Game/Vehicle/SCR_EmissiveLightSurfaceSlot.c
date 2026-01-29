class SCR_EmissiveLightSurfaceSlot : EmissiveLightSurfaceSlot
{
	//------------------------------------------------------------------------------------------------
	override void OnAttachedEntity(IEntity entity)
	{
		super.OnAttachedEntity(entity);
		HitZoneContainerComponent hitZoneContainer = HitZoneContainerComponent.Cast(entity.FindComponent(HitZoneContainerComponent));
		if (!hitZoneContainer)
			return;

		SCR_LightHitZone hitZone = SCR_LightHitZone.Cast(hitZoneContainer.GetDefaultHitZone());
		if (hitZone)
			hitZone.SetSurfaceName(GetSourceName());
	}

	//------------------------------------------------------------------------------------------------
	override void OnDetachedEntity(IEntity entity)
	{
		super.OnDetachedEntity(entity);
		HitZoneContainerComponent hitZoneContainer = HitZoneContainerComponent.Cast(entity.FindComponent(HitZoneContainerComponent));
		if (!hitZoneContainer)
			return;

		SCR_LightHitZone hitZone = SCR_LightHitZone.Cast(hitZoneContainer.GetDefaultHitZone());
		if (hitZone)
			hitZone.SetSurfaceName(string.Empty);
	}
}
