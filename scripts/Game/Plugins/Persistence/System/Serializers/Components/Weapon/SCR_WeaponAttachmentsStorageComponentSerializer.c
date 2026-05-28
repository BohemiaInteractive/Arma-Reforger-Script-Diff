class SCR_BaseInventoryStorageComponentSerializerSlot
{
	[SortAttribute()]
	int m_iNumRequirements = 0;

	BaseInventoryStorageComponentSerializerSlot m_pSlot;
	
	//------------------------------------------------------------------------------------------------
	void SCR_BaseInventoryStorageComponentSerializerSlot(BaseInventoryStorageComponentSerializerSlot slot)
	{
		m_pSlot = slot;

		auto inventoryItemComponent = InventoryItemComponent.Cast(slot.GetEntity().FindComponent(InventoryItemComponent));
		if (!inventoryItemComponent)
			return;

		auto obstructionAttributes = SCR_WeaponAttachmentObstructionAttributes.Cast(inventoryItemComponent.FindAttribute(SCR_WeaponAttachmentObstructionAttributes));
		if (!obstructionAttributes)
			return;

		m_iNumRequirements = obstructionAttributes.GetRequiredAttachmentTypes().Count();
	}
}

class SCR_WeaponAttachmentsStorageComponentSerializer : WeaponAttachmentsStorageComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_WeaponAttachmentsStorageComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected void HandleInsertionOrder(inout array<BaseInventoryStorageComponentSerializerSlot> slots)
	{
		array<ref SCR_BaseInventoryStorageComponentSerializerSlot> sortedSlots();
		foreach (auto slot : slots)
		{
			SCR_BaseInventoryStorageComponentSerializerSlot sortSlot(slot);
			sortedSlots.Insert(sortSlot);
		}
		sortedSlots.Sort();

		slots.Clear();
		foreach (auto sortedSlot : sortedSlots)
		{
			slots.Insert(sortedSlot.m_pSlot);
		}
	}
}
