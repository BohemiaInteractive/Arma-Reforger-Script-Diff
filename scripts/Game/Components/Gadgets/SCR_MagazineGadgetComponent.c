class SCR_MagazineGadgetComponentClass : SCR_GadgetComponentClass
{
}

class SCR_MagazineGadgetComponent : SCR_GadgetComponent
{
	//------------------------------------------------------------------------------------------------
	override protected void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		if (mode != EGadgetMode.IN_STORAGE)
		{
			super.ModeSwitch(mode, charOwner);
			return;
		}

		InventoryMagazineComponent inventoryMagComp = InventoryMagazineComponent.Cast(GetOwner().FindComponent(InventoryMagazineComponent));
		if (!inventoryMagComp)
		{
			super.ModeSwitch(mode, charOwner);
			return;
		}

		InventoryStorageSlot parentSlot = inventoryMagComp.GetParentSlot();
		if (!parentSlot)
		{
			super.ModeSwitch(mode, charOwner);
			return;
		}

		BaseInventoryStorageComponent storage = parentSlot.GetStorage();
		SCR_WeaponAttachmentsStorageComponent weaponStorage = SCR_WeaponAttachmentsStorageComponent.Cast(parentSlot.GetStorage());
		if (!weaponStorage)
		{
			IEntity parentOwner = parentSlot.GetOwner();
			if (parentOwner) // do manual check as storage manager wont be set for this specific slot when weapon is spawned, since there is probably some hidden delay before it is connected
				weaponStorage = SCR_WeaponAttachmentsStorageComponent.Cast(parentOwner.FindComponent(SCR_WeaponAttachmentsStorageComponent));
		}

		if (weaponStorage)
			mode = EGadgetMode.IN_SLOT;

		m_iMode = mode;
		super.ModeSwitch(mode, charOwner);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsVisibleEquipped()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateSlottedPosition(notnull InventoryItemComponent itemComponent, notnull InventoryStorageSlot slot)
	{
		return;
	}
}