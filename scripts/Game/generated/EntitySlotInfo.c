/*
===========================================
Do not modify, this script is generated
===========================================
*/

//! Adds ability to attach an object to a slot
class EntitySlotInfo: PointInfo
{
	private ref ScriptInvokerEntity AttachedEntityInvoker;
	private ref ScriptInvokerEntity DetachedEntityInvoker;
	ScriptInvokerEntity GetAttachedEntityInvoker()
	{
		if (!AttachedEntityInvoker)
			AttachedEntityInvoker = new ScriptInvokerEntity();
		return AttachedEntityInvoker;
	}
	ScriptInvokerEntity GetDetachedEntityInvoker()
	{
		if (!DetachedEntityInvoker)
			DetachedEntityInvoker = new ScriptInvokerEntity();
		return DetachedEntityInvoker;
	}
	//@TODO(Leo): find out why it is needed and refactor to cpp, everything is accessible here
	//! Checks whether provided entity has parent and if so, tries to find a slot which it would belong to
	//! \param entity The slotted entity to get parent slot info for									  
	//! \return Returns Returns slot info of slotted entity												  
	static EntitySlotInfo GetSlotInfo(notnull IEntity entity)											  
	{																									  
		IEntity parent = entity.GetParent();															  
		if (!parent)																					  
			return null;																				  
																										  
		// Check slot managers																			  
		array<Managed> slotManagers = {};																  
		parent.FindComponents(SlotManagerComponent, slotManagers);										  
		foreach (Managed managed : slotManagers)														  
		{																								  
			SlotManagerComponent slotManager = SlotManagerComponent.Cast(managed);						  
			array<EntitySlotInfo> managerSlotInfos = {};												  
			slotManager.GetSlotInfos(managerSlotInfos);													  
			foreach (EntitySlotInfo slotInfo : managerSlotInfos)										  
			{																							  
				if (slotInfo && slotInfo.GetAttachedEntity() == entity)									  
					return slotInfo;																	  
			}																							  
		}																								  
																										  
		// Check individual slot components																  
		array<Managed> slots = {};																		  
		parent.FindComponents(BaseSlotComponent, slots);												  
		foreach (Managed managed : slots)																  
		{																								  
			BaseSlotComponent slot = BaseSlotComponent.Cast(managed);									  
			EntitySlotInfo slotInfo = slot.GetSlotInfo();												  
			if (slotInfo && slotInfo.GetAttachedEntity() == entity)										  
				return slotInfo;																		  
		}																								  
																										  
		// Check weapon slot components																	  
		array<Managed> weaponSlots = {};																  
		parent.FindComponents(WeaponSlotComponent, weaponSlots);										  
		foreach (Managed managed : weaponSlots)															  
		{																								  
			WeaponSlotComponent slot = WeaponSlotComponent.Cast(managed);								  
			EntitySlotInfo slotInfo = slot.GetSlotInfo();												  
			if (slotInfo && slotInfo.GetAttachedEntity() == entity)										  
				return slotInfo;																		  
		}																								  
																										  
		return null;																					  
	}																									  
																										  
	//------------------------------------------------------------------------------------------------	  
	//! Get slots of the passed entity																	  
	//! \param entity The parent entity to get slot infos for											  
	//! \param slotInfos Slot infos array to return slot infos to										  
	//! \return Returns Returns children slot infos of entity											  
	static void GetSlotInfos(notnull IEntity entity, inout notnull array<EntitySlotInfo> slotInfos)		  
	{																									  
		// Get slot infos of slot managers																  
		array<Managed> slotManagers = {};																  
		entity.FindComponents(SlotManagerComponent, slotManagers);										  
		foreach (Managed managed : slotManagers)														  
		{																								  
			SlotManagerComponent slotManager = SlotManagerComponent.Cast(managed);						  
			array<EntitySlotInfo> managerSlotInfos = {};												  
			slotManager.GetSlotInfos(managerSlotInfos);													  
			foreach (EntitySlotInfo slotInfo : managerSlotInfos)										  
			{																							  
				if (slotInfo)																			  
					slotInfos.Insert(slotInfo);															  
			}																							  
		}																								  
																										  
		// Get slot info of individual slot components													  
		array<Managed> slotComponents = {};																  
		entity.FindComponents(BaseSlotComponent, slotComponents);										  
		foreach (Managed managed : slotComponents)														  
		{																								  
			BaseSlotComponent slot = BaseSlotComponent.Cast(managed);									  
			EntitySlotInfo slotInfo = slot.GetSlotInfo();												  
			if (slotInfo)																				  
				slotInfos.Insert(slotInfo);																  
		}																								  
																										  
		// Get slot info of individual weapon slot components											  
		array<Managed> weaponSlotComponents = {};														  
		entity.FindComponents(WeaponSlotComponent, weaponSlotComponents);								  
		foreach (Managed managed : weaponSlotComponents)												  
		{																								  
			WeaponSlotComponent slot = WeaponSlotComponent.Cast(managed);								  
			EntitySlotInfo slotInfo = slot.GetSlotInfo();												  
			if (slotInfo)																				  
				slotInfos.Insert(slotInfo);																  
		}																								  
	}																									  

	proto external bool IsEnabled();
	//! Returns currently attached entity or null if none
	proto external IEntity GetAttachedEntity();
	//! Returns the name of the EntitySlotInfo defined in the array of slots. If empty will return the store name instead.
	proto external string GetSourceName();
	proto external ResourceName GetSlotTemplate();
	/*!
	Updates attached entity's transformation by combining provided transformation with slot predefined local transformation
	Once new entity is attached to slot provided transformation is discarded and predifined local transformation is used instead
	*/
	proto external void SetAdditiveTransformLS(vector matLS[4]);
	/*!
	Overrides local transformation of entity
	Once new entity is attached to slot provided transformation is discarded and predifined local transformation is used instead
	*/
	proto external void OverrideTransformLS(vector matLS[4]);
	/*!
	Detaches entity from this slot info (if any)
	\param physicalChange Do update hierarchy?
	*/
	proto external void DetachEntity(bool physicalChange = true);
	/*!
	Attaches provided entity to this slot info
	Deletes previously attached entity (if any)
	\param childEntity The entity to attach
	*/
	proto external void AttachEntity(IEntity entity);

	// callbacks

	//! Runs every time an entity is attached to the slot.
	event void OnAttachedEntity(IEntity entity) { if (AttachedEntityInvoker) AttachedEntityInvoker.Invoke(entity); };
	//! Runs every time an entity is detached from the slot.
	event void OnDetachedEntity(IEntity entity) { if (DetachedEntityInvoker) DetachedEntityInvoker.Invoke(entity); };
}
