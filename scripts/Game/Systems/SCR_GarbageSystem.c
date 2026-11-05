//! Script entry for garbage system modding
class SCR_GarbageSystem : GarbageSystem
{
	[Attribute(defvalue: "0.25", desc: "Multiplier applied to the initial amount of lifetime of empty magazines", params: "0 inf 0.01")]
	protected float m_fEmptyMagazineInitialLiftimeMultiplier;

	protected const float RESOURCE_LIFETIME_PERCENTAGE = 0.25;

	//------------------------------------------------------------------------------------------------
	override protected float OnInsertRequested(IEntity entity, float lifetime)
	{
		// If a something has at least 25% of its supply capacity double the lifetime.
		const SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(entity);
		if (resourceComponent)
		{
			const SCR_ResourceContainer container = resourceComponent.GetContainer(EResourceType.SUPPLIES);
			if (container && (container.GetMaxResourceValue() > 0))
			{
				if ((container.GetResourceValue() / container.GetMaxResourceValue()) > RESOURCE_LIFETIME_PERCENTAGE)
				{
					lifetime *= 2;
				}
				else // Subscribe for changes that might increase the lifetime later
				{
					container.GetOnResourcesChanged().Insert(HandleVehicleResourcesChanged);
				}
			}
		}

		const MagazineComponent magazine = MagazineComponent.Cast(entity.FindComponent(MagazineComponent));
		if (magazine)
			HandleMagazine(magazine, lifetime);

		return lifetime;
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleMagazine(notnull MagazineComponent magazine, inout float lifetime)
	{
		if (magazine.GetAmmoCount() == 0)
			lifetime *= m_fEmptyMagazineInitialLiftimeMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleVehicleResourcesChanged(SCR_ResourceContainer container)
	{
		const float max = container.GetMaxResourceValue();
		if ((max > 0) && ((container.GetResourceValue() / max) > RESOURCE_LIFETIME_PERCENTAGE))
		{
			container.GetOnResourcesChanged().Remove(HandleVehicleResourcesChanged);
			const IEntity vehicle = container.GetOwner().GetRootParent();
			Bump(vehicle, GetLifetime(vehicle));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceContainer FindVehicleResourceContainer(notnull IEntity entity, EResourceType resourceType)
	{
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
		if (!resourceComponent)
		{
			SlotManagerComponent slotManager = SlotManagerComponent.Cast(entity.FindComponent(SlotManagerComponent));
			if (!slotManager)
				return null;

			EntitySlotInfo slot = slotManager.GetSlotByName("Cargo");
			if (!slot)
				return null;

			entity = slot.GetAttachedEntity();
			if (!entity)
				return null;

			resourceComponent = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
		}

		if (!resourceComponent)
			return null;

		return resourceComponent.GetContainer(resourceType);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_GarbageSystem GetByEntityWorld(IEntity entity)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(entity.GetWorld());
		if (!world)
			return null;

		return SCR_GarbageSystem.Cast(world.GetGarbageSystem());
	}
}
