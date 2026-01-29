[EntityEditorProps(category: "GameScripted/UI/Inventory")]
class SCR_VehicleInventoryStorageManagerComponentClass : ScriptedInventoryStorageManagerComponentClass
{
}

class SCR_VehicleInventoryStorageManagerComponent : ScriptedInventoryStorageManagerComponent
{
	//------------------------------------------------------------------------------------------------
	protected override void FillInitialStorages(out array<BaseInventoryStorageComponent> storagesToAdd)
	{
		super.FillInitialStorages(storagesToAdd);

		if (!GetGame().InPlayMode())
			return;

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		foreach (EntitySlotInfo info : slots)
		{
			info.GetAttachedEntityInvoker().Insert(OnEntityAttached);
			
			IEntity ent = info.GetAttachedEntity();
			if (!ent)
				continue;

			BaseInventoryStorageComponent invComp = BaseInventoryStorageComponent.Cast(ent.FindComponent(BaseInventoryStorageComponent));
			if (invComp)
			{
				storagesToAdd.Insert(invComp);
			}
		}
	}
	
	protected void OnEntityAttached(IEntity ent)
	{
		BaseInventoryStorageComponent invComp = BaseInventoryStorageComponent.Cast(ent.FindComponent(BaseInventoryStorageComponent));
		if (invComp)
		{
			AddStorage(invComp);
		}
	}
	
	protected override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		foreach (EntitySlotInfo info : slots)
		{
			info.GetAttachedEntityInvoker().Remove(OnEntityAttached);
		}
	}
}
