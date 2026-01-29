class SCR_CampaignMHQLockComponentClass : SCR_BaseLockComponentClass
{
}

class SCR_CampaignMHQLockComponent : SCR_BaseLockComponent
{
	protected SCR_CampaignMobileAssemblyComponent m_MHQComponent;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CampaignMobileAssemblyComponent GetMHQComponent()
	{
		if (m_MHQComponent)
			return m_MHQComponent;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
		
		if (!slotManager)
			return null;
		
		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);
		
		foreach (EntitySlotInfo slot : slots)
		{
			if (!slot)
				continue;
			
			IEntity truckBed = slot.GetAttachedEntity();
			
			if (!truckBed)
				continue;
			
			m_MHQComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));
			
			if (m_MHQComponent)
				break;
		}
		
		return m_MHQComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	override LocalizedString GetCannotPerformReason(IEntity user)
	{
		SCR_CampaignMobileAssemblyComponent comp = GetMHQComponent();
		
		if (comp && comp.IsDeployed())
			return "#AR-Campaign_MobileAssemblyDeployed-UC";
		else
			return super.GetCannotPerformReason(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsLocked(IEntity user, BaseCompartmentSlot compartmentSlot)
	{
		if (super.IsLocked(user, compartmentSlot))
			return true;
		
		SCR_CampaignMobileAssemblyComponent comp = GetMHQComponent();
		
		if (comp)
			return comp.IsDeployed();
		
		return false;
	}
}
