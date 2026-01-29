class SCR_WeaponAttachmentsStorageComponentClass: WeaponAttachmentsStorageComponentClass
{
}

class SCR_WeaponAttachmentsStorageComponent : WeaponAttachmentsStorageComponent
{
	ref ScriptInvoker m_OnItemAddedToSlotInvoker = new ScriptInvoker();
	ref ScriptInvoker m_OnItemRemovedFromSlotInvoker = new ScriptInvoker();

	protected ref set<typename> m_aActiveAttachmentTypes = new set<typename>();

	//------------------------------------------------------------------------------------------------
	override bool ShouldHideInVicinity()
	{
		return IsLocked();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanStoreItem(IEntity item, int slotID)
	{
		InventoryItemComponent inventoryItemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!inventoryItemComponent)
			return true;

		SCR_WeaponAttachmentObstructionAttributes obstructionAttributes = SCR_WeaponAttachmentObstructionAttributes.Cast(inventoryItemComponent.FindAttribute(SCR_WeaponAttachmentObstructionAttributes));
		if (!obstructionAttributes)
			return true;

		set<typename> requiredAttachmentTypes = obstructionAttributes.GetRequiredAttachmentTypes();
		foreach (typename attachmentType : requiredAttachmentTypes)
		{
			if (!m_aActiveAttachmentTypes.Contains(attachmentType))
				return false;
		}
		
		set<typename> obstructedAttachmentTypes = obstructionAttributes.GetObstructedAttachmentTypes();
		foreach (typename attachmentType : obstructedAttachmentTypes)
		{
			if (m_aActiveAttachmentTypes.Contains(attachmentType))
				return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Callback when item is added (will be performed locally after server completed the Insert/Move operation)
	override protected void OnAddedToSlot(IEntity item, int slotID)
	{
		m_OnItemAddedToSlotInvoker.Invoke(item, slotID);
		
		InventoryItemComponent inventoryItemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!inventoryItemComponent)
			return;
		
		SCR_WeaponAttachmentObstructionAttributes obstructionAttributes = SCR_WeaponAttachmentObstructionAttributes.Cast(inventoryItemComponent.FindAttribute(SCR_WeaponAttachmentObstructionAttributes));
		if (!obstructionAttributes)
			return;
		
		typename attachmentType = obstructionAttributes.GetAttachmentType().Type();
		m_aActiveAttachmentTypes.Insert(attachmentType);
	}
	
	//------------------------------------------------------------------------------------------------
	// Callback when item is removed (will be performed locally after server completed the Remove/Move operation)
	override protected void OnRemovedFromSlot(IEntity item, int slotID)
	{
		if (!item.IsDeleted())
			m_OnItemRemovedFromSlotInvoker.Invoke(item, slotID);
		
		InventoryItemComponent inventoryItemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!inventoryItemComponent)
			return;
		
		SCR_WeaponAttachmentObstructionAttributes obstructionAttributes = SCR_WeaponAttachmentObstructionAttributes.Cast(inventoryItemComponent.FindAttribute(SCR_WeaponAttachmentObstructionAttributes));
		if (!obstructionAttributes)
			return;
		
		typename attachmentType = obstructionAttributes.GetAttachmentType().Type();
		m_aActiveAttachmentTypes.RemoveItem(attachmentType);

		RemoveNestedAttachments(attachmentType);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveNestedAttachments(typename removedAttachmentType)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;
		
		InventoryStorageManagerComponent storageManager = InventoryStorageManagerComponent.Cast(character.FindComponent(InventoryStorageManagerComponent));
		if (!storageManager)
			return;
			
		array<IEntity> storedItems = {};
		GetAll(storedItems);	
		foreach (IEntity storedItem : storedItems)
		{
			InventoryItemComponent inventoryItemComponent = InventoryItemComponent.Cast(storedItem.FindComponent(InventoryItemComponent));
			if (!inventoryItemComponent)
				continue;
			
			SCR_WeaponAttachmentObstructionAttributes obstructionAttributes = SCR_WeaponAttachmentObstructionAttributes.Cast(inventoryItemComponent.FindAttribute(SCR_WeaponAttachmentObstructionAttributes));
			if (!obstructionAttributes)
				continue;
			
			set<typename> requiredAttachmentTypes = obstructionAttributes.GetRequiredAttachmentTypes();
			if (requiredAttachmentTypes && requiredAttachmentTypes.Contains(removedAttachmentType))
			{
				BaseInventoryStorageComponent storage = storageManager.FindStorageForItem(storedItem);		
				if (storage)
					storageManager.TryMoveItemToStorage(storedItem, storage);
			}
		}
	}
}
