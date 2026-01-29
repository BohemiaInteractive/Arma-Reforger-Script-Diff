class SCR_WeaponStatsManagerComponentClass: BaseWeaponStatsManagerComponentClass
{
}

class SCR_WeaponStatsManagerComponent: BaseWeaponStatsManagerComponent
{

	override void OnWeaponAttachedAttachment(IEntity entity, int iMuzzleIndex)
	{
		InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(entity.FindComponent(InventoryItemComponent));
		if (inventoryItem)
		{
			ItemAttributeCollection attribs = inventoryItem.GetAttributes();
			// Is there a way to find all of them?
			SCR_WeaponAttachmentAttributes waa = SCR_WeaponAttachmentAttributes.Cast(attribs.FindAttribute(SCR_WeaponAttachmentAttributes));
			if (waa)
			{
				if (!waa.ApplyModifiers(this, iMuzzleIndex, entity))
				{
					Print("WARNING: One or more override stats failed to set", LogLevel.ERROR);
					Print(string.Format("Entity: %1, Attachment Entity %2", GetOwner(), entity), LogLevel.ERROR);
				}
			}
		}		
	}
	
	override void OnWeaponDetachedAttachment(IEntity entity, int iMuzzleIndex)
	{
		InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(entity.FindComponent(InventoryItemComponent));
		if (inventoryItem)
		{
			ItemAttributeCollection attribs = inventoryItem.GetAttributes();
			// Is there a way to find all of them?
			SCR_WeaponAttachmentAttributes waa = SCR_WeaponAttachmentAttributes.Cast(attribs.FindAttribute(SCR_WeaponAttachmentAttributes));
			if (waa)
			{
				waa.ClearModifiers(this, iMuzzleIndex, entity);
			}
		}
	}
}