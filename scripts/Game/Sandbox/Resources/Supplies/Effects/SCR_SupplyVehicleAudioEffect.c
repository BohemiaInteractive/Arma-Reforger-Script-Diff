[BaseContainerProps()]
class SCR_SupplyVehicleAudioEffect : SCR_SupplyEffectBase
{
	[Attribute("SOUND_SUPPLIES_VEH_CONTAINER_APPEAR")]
	protected string m_sSuppliesAppearSound;

	[Attribute("SOUND_SUPPLIES_VEH_CONTAINER_DISAPPEAR")]
	protected string m_sSuppliesDisappearSound;

	[Attribute("SOUND_SUPPLIES_PARTIAL_LOAD")]
	protected string m_sSuppliesPartialLoadSound;

	[Attribute("SOUND_SUPPLIES_PARTIAL_UNLOAD")]
	protected string m_sSuppliesPartialUnloadSound;

	[Attribute("SOUND_SUPPLIES_VEH_FULL")]
	protected string m_sSuppliesFullSound;

	//------------------------------------------------------------------------------------------------
	override void ActivateEffect(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		if (interactionType == EResourcePlayerInteractionType.VEHICLE_LOAD)
		{
			SCR_VehicleSoundComponent vehicleSoundComp = SCR_VehicleSoundComponent.Cast(resourceComponentTo.GetOwner().GetRootParent().FindComponent(SCR_VehicleSoundComponent));
			if (!vehicleSoundComp)
				return;

			PlaySupplyLoadSound(vehicleSoundComp, resourceComponentTo, resourceType, resourceValue);
		}
		else if (interactionType == EResourcePlayerInteractionType.VEHICLE_UNLOAD)
		{
			SCR_VehicleSoundComponent vehicleSoundComp = SCR_VehicleSoundComponent.Cast(resourceComponentTo.GetOwner().GetRootParent().FindComponent(SCR_VehicleSoundComponent));
			if (!vehicleSoundComp)
				return;

			PlaySupplyUnloadSound(vehicleSoundComp, resourceComponentTo, resourceType, resourceValue);		
		}
		else if (interactionType == EResourcePlayerInteractionType.INVENTORY_SPLIT)
		{
			SCR_VehicleSoundComponent vehicleSoundComp = SCR_VehicleSoundComponent.Cast(resourceComponentFrom.GetOwner().GetRootParent().FindComponent(SCR_VehicleSoundComponent));
			if (vehicleSoundComp)
			{
				PlaySupplyUnloadSound(vehicleSoundComp, resourceComponentFrom, resourceType, resourceValue);
			}
			else
			{
				vehicleSoundComp = SCR_VehicleSoundComponent.Cast(resourceComponentTo.GetOwner().GetRootParent().FindComponent(SCR_VehicleSoundComponent));
				if (!vehicleSoundComp)
					return;

				PlaySupplyLoadSound(vehicleSoundComp, resourceComponentTo, resourceType, resourceValue);
			}
		}
	}

	//------------------------------------------------------------------------------------------------	
	protected void PlaySupplyLoadSound(SCR_VehicleSoundComponent vehicleSoundComp, SCR_ResourceComponent resourceComp, EResourceType resourceType, float resourceValue)
	{
		float current, max;
		SCR_ResourceSystemHelper.GetStoredAndMaxResources(resourceComp, current, max, resourceType);

		if (current - resourceValue == 0)
			vehicleSoundComp.SoundEvent(m_sSuppliesAppearSound);
		else if (current == max)
			vehicleSoundComp.SoundEvent(m_sSuppliesFullSound);
		else
			vehicleSoundComp.SoundEvent(m_sSuppliesPartialLoadSound);
	}

	//------------------------------------------------------------------------------------------------
	protected void PlaySupplyUnloadSound(SCR_VehicleSoundComponent vehicleSoundComp, SCR_ResourceComponent resourceComp, EResourceType resourceType, float resourceValue)
	{
		float current, max;
		SCR_ResourceSystemHelper.GetStoredAndMaxResources(resourceComp, current, max, resourceType);

		if (current == 0)
			vehicleSoundComp.SoundEvent(m_sSuppliesDisappearSound);
		else
			vehicleSoundComp.SoundEvent(m_sSuppliesPartialUnloadSound);	
	}
}
