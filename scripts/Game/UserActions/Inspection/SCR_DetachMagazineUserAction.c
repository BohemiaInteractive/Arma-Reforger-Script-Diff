class SCR_DetachMagazineUserAction : SCR_InspectionUserAction
{
	protected SCR_InventoryStorageManagerComponent m_InventoryManager;
	
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;
		
		if(!m_InventoryManager)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
			if(!character)
				return false;
			
			m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		}
			
		if(!m_InventoryManager || !m_WeaponComponent.GetCurrentMagazine())
			return false;
		
		BaseMuzzleComponent muzzleComp = m_WeaponComponent.GetCurrentMuzzle();
		if (GetOwner() != muzzleComp.GetOwner())
			return false;
		
		IEntity currentMag = m_WeaponComponent.GetCurrentMagazine().GetOwner();
		InventoryItemComponent magInventory = InventoryItemComponent.Cast(currentMag.FindComponent(InventoryItemComponent));
		BaseInventoryStorageComponent magStorage = magInventory.GetParentSlot().GetStorage();
		WeaponAttachmentsStorageComponent wasc = WeaponAttachmentsStorageComponent.Cast(magStorage);
		if (!wasc)
			return false; // Must be a WeaponAttachmentsStorageComponent

		return m_InventoryManager.CanRemoveItemFromStorage(currentMag, magStorage);
	}

	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		IEntity currentMag = m_WeaponComponent.GetCurrentMagazine().GetOwner();
		InventoryItemComponent magInventory = InventoryItemComponent.Cast(currentMag.FindComponent(InventoryItemComponent));
		BaseInventoryStorageComponent magStorage = magInventory.GetParentSlot().GetStorage();
		
		WeaponAttachmentsStorageComponent wasc = WeaponAttachmentsStorageComponent.Cast(magStorage);
		if (!wasc)
		{
			Print("ERROR: Magazine is no longer in the weapon", LogLevel.ERROR);
			return; // Must be a WeaponAttachmentsStorageComponent
		}
		
		auto charCtrl = CharacterControllerComponent.Cast(m_InventoryManager.GetOwner().FindComponent(CharacterControllerComponent));
		charCtrl.DetachCurrentMagazine();
		
		m_InventoryManager.PlayItemSound(pOwnerEntity, "SOUND_UNEQUIP");		
	}

	override bool GetActionNameScript(out string outName)
	{
		if (!m_WeaponComponent)
			return false;

		if (!m_WeaponComponent.GetCurrentMagazine())
			return false;

		IEntity currentMag = m_WeaponComponent.GetCurrentMagazine().GetOwner();
		if (!currentMag)
			return false;

		InventoryItemComponent magItem = InventoryItemComponent.Cast(currentMag.FindComponent(InventoryItemComponent));
		if (!magItem)
			return false;

		UIInfo actionInfo = GetUIInfo();
		UIInfo itemInfo = magItem.GetUIInfo();
		if (actionInfo && itemInfo)
		{
			outName = string.Format("%1%2", actionInfo.GetName(), itemInfo.GetName());
			return true;
		}
		else
		{
			return false;
		}
	}
};