class SCR_SlotServiceComponentClass : SCR_ServicePointComponentClass
{
	[Attribute(defvalue: "5", params: "0 inf", desc: "Slot Search Radius Size.", category: "Slot service")]
	protected float m_fMaxSlotDistance;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMaxSlotDistance()
	{
		return m_fMaxSlotDistance;
	}
}

//! Service with basic slot handling functionalities. 
class SCR_SlotServiceComponent : SCR_ServicePointComponent
{
	protected SCR_SpawnerSlotManager m_SlotManager;
	protected ref array<SCR_EntitySpawnerSlotComponent> m_aChildSlots = {};
	protected ref array<SCR_EntitySpawnerSlotComponent> m_aNearSlots = {};
	
	//------------------------------------------------------------------------------------------------
	//! Register slot to be used for service.
	//! Slots that do not meet conditions in CanBeSlotRegistered are ignored, so are already known slots.
	//! \param[in] slot Slot to be registered
	void RegisterSlot(SCR_EntitySpawnerSlotComponent slot)
	{
		if (!CanBeSlotRegistered(slot))
			return;
		
		if (!m_aNearSlots.Contains(slot))
			m_aNearSlots.Insert(slot);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prevents registration of slots that are too far from service, or are already child of another slot service
	//! \param[in] slot
	//! \return
	protected bool CanBeSlotRegistered(notnull SCR_EntitySpawnerSlotComponent slot)
	{
		IEntity slotOwner = slot.GetOwner();
		IEntity parent = slotOwner.GetParent();
		if (parent && parent.FindComponent(SCR_SlotServiceComponent) && parent != GetOwner())
			return false;
		
		SCR_SlotServiceComponentClass prefabData = SCR_SlotServiceComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return false;
		
		float maxSlotDistance = prefabData.GetMaxSlotDistance();
		return vector.DistanceSqXZ(slotOwner.GetOrigin(), GetOwner().GetOrigin()) < maxSlotDistance * maxSlotDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback function for Query used in RegisterNearbySlots
	//! Should callback return true, QueryEntitiesBySphere will continue with query. If false, it will stop.
	protected bool SlotSearchCallback(IEntity ent)
	{
		SCR_EntitySpawnerSlotComponent slotComp = SCR_EntitySpawnerSlotComponent.Cast(ent.FindComponent(SCR_EntitySpawnerSlotComponent));
		if (!slotComp)
			return true;
		
		RegisterSlot(slotComp);
		return true;
 	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers slots in near distance. Kept for sake of client checks. Skips slots that are in hiearchy of any entity with SCR_SlotService or classes inherited from it
	protected void RegisterNearbySlots()
	{
		SCR_SlotServiceComponentClass prefabData = SCR_SlotServiceComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;
		
		GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), prefabData.GetMaxSlotDistance(), SlotSearchCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers slots initially created as children. There slots cannot be used by any other spawner and shouldn't change in runtime.
	protected void RegisterChildSlots()
	{
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			SCR_EntitySpawnerSlotComponent slot = SCR_EntitySpawnerSlotComponent.Cast(child.FindComponent(SCR_EntitySpawnerSlotComponent));
			if (slot)
				m_aChildSlots.Insert(slot);
			
			child = child.GetSibling();			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called, if slot possition was changed. If it fails to meet previously required criteria in CanBeSlotRegistered, slot wont be used anymore
	protected void OnSlotUpdate(SCR_EntitySpawnerSlotComponent slot, vector position)
	{
		if (CanBeSlotRegistered(slot))
		{
			if (!m_aNearSlots.Contains(slot) || m_aChildSlots.Contains(slot))
				m_aNearSlots.Insert(slot);
		}
		else
		{
			m_aNearSlots.RemoveItem(slot);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called, if slot possition was changed. If it fails to meet previously required criteria in CanBeSlotRegistered, slot wont be used anymore
	protected void OnSlotRemoved(SCR_EntitySpawnerSlotComponent slot)
	{
		m_aNearSlots.RemoveItem(slot);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		m_SlotManager = SCR_SpawnerSlotManager.GetInstance();
		if (!m_SlotManager)
		{
			Print("Slot manager is required for Slot service functionality", LogLevel.ERROR);
			return;
		}
		
		RegisterChildSlots();
		RegisterNearbySlots();
		
		m_SlotManager.GetOnSlotCreated().Insert(RegisterSlot);
		m_SlotManager.GetOnSlotUpdated().Insert(OnSlotUpdate);
		m_SlotManager.GetOnSlotRemoved().Insert(OnSlotRemoved);
	}
}
