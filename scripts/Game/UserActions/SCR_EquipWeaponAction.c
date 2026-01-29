//------------------------------------------------------------------------------------------------
//! Equip weapon
class SCR_EquipWeaponAction : SCR_BaseWeaponAction
{
	#ifndef DISABLE_INVENTORY

	[Attribute( "#AR-Inventory_Equip", desc: "What text should be displayed when the Equip action is available?" )]
	protected string m_sEquipActionString;
	[Attribute( "#AR-Inventory_Replaces", desc: "What text should be displayed when the Replace action is available?" )]
	protected string m_sReplaceActionString;
	
	int m_iTargetSlotID = -1;
	bool m_bWasUnequipping = false;
	ref SCR_EquipPickedWeapon m_pEquipCB = new SCR_EquipPickedWeapon;
	ref SCR_RemoveEquippedWeapon m_pRemoveWeaponCB = new SCR_RemoveEquippedWeapon;
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirmed(IEntity pUserEntity)
	{
		// If we were unequipping weapon, let's now play pickup gesture and callback back into this method
		if (m_bWasUnequipping)
		{
			m_bWasUnequipping = false;
			CharacterControllerComponent controller = CharacterControllerComponent.Cast(pUserEntity.FindComponent(CharacterControllerComponent));
			// If we were unable to play gesture, something happened to character and it cannot equip next weapon now, so make sure to perform cleanup
			if (!controller.TryPlayItemGesture(EItemGesture.EItemGesturePickUp, this, "Character_ActionGrab"))
			{
				m_iTargetSlotID = -1;
			}
			return;
		}
		IEntity pOwnerEntity = m_Item.GetOwner();
		SCR_InventoryStorageManagerComponent manager = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		EquipedWeaponStorageComponent weaponStorage = EquipedWeaponStorageComponent.Cast(pUserEntity.FindComponent(EquipedWeaponStorageComponent));

		if (!manager || !weaponStorage)
			return;
		m_pEquipCB.m_pWeapon = pOwnerEntity;
		
		// Double check actual state... By this time slot shouldn't contain any weapon
		if (weaponStorage.GetSlot(m_iTargetSlotID) == null || weaponStorage.GetSlot(m_iTargetSlotID).GetAttachedEntity() != null)
		{
			m_iTargetSlotID = -1;
			return;
		}
		if (manager.TryInsertItemInStorage(pOwnerEntity, weaponStorage, m_iTargetSlotID, m_pEquipCB))
		{
			// Play sound
			manager.PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_EQUIP);
		}
		
		m_iTargetSlotID = -1;
	}
	//------------------------------------------------------------------------------------------------
	//! just a cleanup in case of interruption
	override void OnRejected(IEntity pUserEntity)
	{
		m_bWasUnequipping = false;
		m_iTargetSlotID = -1;
	}
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CharacterControllerComponent controller = CharacterControllerComponent.Cast(pUserEntity.FindComponent(CharacterControllerComponent));
		if (!controller || controller.IsPlayingGesture())
			return;
		if (m_iTargetSlotID > -1)
			return;
		m_pEquipCB.m_pController = controller;
		EquipedWeaponStorageComponent weaponStorage = EquipedWeaponStorageComponent.Cast(pUserEntity.FindComponent(EquipedWeaponStorageComponent));
		
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(pUserEntity.FindComponent(BaseWeaponManagerComponent));
		
		if (!weaponStorage || !weaponManager)
			return;
		
		WeaponComponent weapon = WeaponComponent.Cast(pOwnerEntity.FindComponent(WeaponComponent));
		WeaponSlotComponent weaponSlotCurr = weaponManager.GetCurrentSlot();
		
		array<WeaponSlotComponent> weaponsList = {};
		int weaponCount = weaponManager.GetWeaponsSlots(weaponsList);
		array<int> suitableSlots = {};
		// Just filtering out suitable slots by weapon type
		for (int i = 0; i < weaponCount; i++)
		{
			WeaponSlotComponent weaponSlot = weaponsList[i];
			string weaponSlotType = weaponSlot.GetWeaponSlotType();
			
			if (weaponSlotType.Compare(weapon.GetWeaponSlotType()) != 0 && !CharacterHandWeaponSlotComponent.Cast(weaponSlot))
			{
				continue;
			}
			suitableSlots.Insert(i);
		}
		
		if (suitableSlots.Count() == 0)
			return;
		
		int i = 0;
		// Let's try to find empty slot and equip weapon to it
		if (suitableSlots.Count() > 1)
		{
			for (int j = 0, countJ = suitableSlots.Count(); j < countJ; j++)
			{
				WeaponSlotComponent slot = weaponsList[suitableSlots[j]];
				
				if (slot.GetWeaponEntity() == null)
				{
					i = j;
					break;
				}
				// or fall back to currently equipped
				if (slot == weaponSlotCurr)
				{
					i = j;
				}
			}
		}
		WeaponSlotComponent slot = weaponsList[suitableSlots[i]];
		// Target slot has weapon, first detach it
		if (slot.GetWeaponEntity())
		{
			m_pRemoveWeaponCB.m_pController = controller;
			m_pRemoveWeaponCB.m_pParentAction = this;
			m_iTargetSlotID = slot.GetWeaponSlotIndex();
			// Say to callback that we want to immediately place weapon in hands after grab if target slot is currently selected weapon, if it is not then play full cycle of change weapon
			// situation when we want to play full cycle might happen when we have let's say rifle in hands and we want to equip handgun from the ground
			m_pEquipCB.m_bNoEquipAnims = weaponSlotCurr == slot;
			m_bWasUnequipping = true;
			if (!manager.TryRemoveItemFromStorage(slot.GetWeaponEntity(), weaponStorage, m_pRemoveWeaponCB))
			{
				m_iTargetSlotID = -1;
				m_bWasUnequipping = false;
				m_pEquipCB.m_bNoEquipAnims = false;
			}
			return;
		}
		m_bWasUnequipping = false;
		m_pEquipCB.m_bNoEquipAnims = weaponSlotCurr == null;
		// Free hands, play gesture right away and grab weapon
		// If we are equiping to hand slot, just move it into hand directly
		if (CharacterHandWeaponSlotComponent.Cast(slot))
		{
			if (manager.TryInsertItemInStorage(pOwnerEntity, weaponStorage, slot.GetWeaponSlotIndex()))
			{
				// Play sound
				manager.PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_EQUIP);
			}
		}
		else
		{
			if (controller.TryPlayItemGesture(EItemGesture.EItemGesturePickUp, this, "Character_ActionGrab"))
				m_iTargetSlotID = slot.GetWeaponSlotIndex();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		if ( !super.CanBePerformedScript( user ) )
			return false;
		CharacterControllerComponent controller = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		if (m_pWeaponOnGroundComponent.CanBeEquipped(controller) == ECanBeEquippedResult.STANCE_TOO_LOW)
		{
			SetCannotPerformReason("#AR-Inventory_StanceTooLow");
			return false;
		}
		
		return CanEquipOrReplaceWeapon( user ) && !controller.IsReloading() && controller.CanPlayItemGesture();

 	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (m_bIsSwappingWeapons)
		{
			string replaceAction = string.Format(WidgetManager.Translate(m_sReplaceActionString), WidgetManager.Translate(m_sWeaponToSwapName));
			outName = string.Format("%1 %2", m_sEquipActionString, replaceAction);
		}
		else
			outName = m_sEquipActionString;		
		
		return true;
	}
 
	#endif
};

//------------------------------------------------------------------------------------------------
class SCR_EquipPickedWeapon : ScriptedInventoryOperationCallback
{
	CharacterControllerComponent m_pController;
	IEntity m_pWeapon;
	bool m_bNoEquipAnims;
	protected override void OnComplete()
	{
		m_pController.TryEquipRightHandItem(m_pWeapon, EEquipItemType.EEquipTypeWeapon, m_bNoEquipAnims);
	}
	
};

//------------------------------------------------------------------------------------------------
class SCR_RemoveEquippedWeapon : ScriptedInventoryOperationCallback
{
	SCR_EquipWeaponAction m_pParentAction;
	CharacterControllerComponent m_pController;
	protected override void OnComplete()
	{
		m_pParentAction.OnConfirmed(m_pController.GetOwner());
	}
	
};

//------------------------------------------------------------------------------------------------
//! Pickup weapon to holster position, without equipping it
class SCR_EquipWeaponHolsterAction : SCR_BaseWeaponAction
{
	#ifndef DISABLE_INVENTORY
		
	[Attribute( "#AR-Inventory_PickUp", desc: "What text should be displayed when the Pick up action is available?" )]
	protected string m_sPickUpActionString;
	[Attribute( "#AR-Inventory_Replaces", desc: "What text should be displayed when the Replace text is visible?" )]
	protected string m_sReplaceActionString;
	[Attribute( "#AR-Inventory_EquipSling", desc: "What text should be displayed when the picked up weapons type is a primary" )]
	protected string m_sEquipingSlingString;
	[Attribute( "#AR-Inventory_EquipHolster", desc: "What text should be displayed when the picked up weapons type is a secondary" )]
	protected string m_sEquipingHolsterString;
	
	int m_iTargetSlotID = -1;
	
	override void OnConfirmed(IEntity pUserEntity)
	{
		IEntity pOwnerEntity = m_Item.GetOwner();
		SCR_InventoryStorageManagerComponent manager = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		EquipedWeaponStorageComponent weaponStorage = EquipedWeaponStorageComponent.Cast(pUserEntity.FindComponent(EquipedWeaponStorageComponent));

		if (!manager || !weaponStorage)
			return;
		
		if (weaponStorage.GetSlot(m_iTargetSlotID) != null)
		{
			if (weaponStorage.GetSlot(m_iTargetSlotID).GetAttachedEntity() != null)
			{
				manager.TryReplaceItem(pOwnerEntity, weaponStorage, m_iTargetSlotID);
			}
			else
			{
				manager.TryInsertItemInStorage(pOwnerEntity, weaponStorage, m_iTargetSlotID);
			}
		}
		m_iTargetSlotID = -1;
		// Play sound
		manager.PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_EQUIP);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CharacterControllerComponent controller = CharacterControllerComponent.Cast(pUserEntity.FindComponent(CharacterControllerComponent));
		if (!controller || controller.IsChangingItem() || controller.IsPlayingGesture())
			return;
		if (m_iTargetSlotID > -1)
			return;
		
		EquipedWeaponStorageComponent weaponStorage = EquipedWeaponStorageComponent.Cast(pUserEntity.FindComponent(EquipedWeaponStorageComponent));

		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(pUserEntity.FindComponent(BaseWeaponManagerComponent));
		
		if (!weaponStorage || !weaponManager)
			return;
		
		WeaponComponent weapon = WeaponComponent.Cast(pOwnerEntity.FindComponent(WeaponComponent));
		WeaponSlotComponent weaponSlotCurr = weaponManager.GetCurrentSlot();
		
		array<WeaponSlotComponent> weaponsList = {};
		int weaponCount = weaponManager.GetWeaponsSlots(weaponsList);
		array<int> suitableSlots = {};
		for (int i = 0; i < weaponCount; i++)
		{
			WeaponSlotComponent weaponSlot = weaponsList[i];
			string weaponSlotType = weaponSlot.GetWeaponSlotType();
			
			if (weaponSlotType.Compare(weapon.GetWeaponSlotType()) != 0 && !CharacterHandWeaponSlotComponent.Cast(weaponSlot))
			{
				continue;
			}
			suitableSlots.Insert(i);
		}
		
		if (suitableSlots.Count() == 0)
			return;
		
		WeaponSlotComponent weaponSlotFinal = null;
		// Default is first found slot
		int i = 0;
		// There are several slots that can held current weapon type
		if (suitableSlots.Count() > 1)
		{
			for (int j = 0; j < suitableSlots.Count(); j++)
			{
				// If we have weapon equipped, search for the slot that is not selected one
				if (weaponSlotCurr != null)
				{
					if (weaponSlotCurr != weaponsList[suitableSlots[j]])
					{
						i = j;
						break;
					}
				}
				else
				{
					// otherwise prefer slot that doesn't have any weapon set in it
					WeaponSlotComponent slotComp = weaponsList[suitableSlots[j]];
					if (!slotComp.GetWeaponEntity())
					{
						i = j;
						break;
					}
				}
			}
		}
		
		weaponSlotFinal = weaponsList[suitableSlots[i]];
		
		if (weaponSlotFinal == weaponSlotCurr && weaponSlotCurr != null && weaponSlotCurr.GetWeaponEntity() != null)
			return;
		
		if (controller.TryPlayItemGesture(EItemGesture.EItemGesturePickUp, this, "Character_ActionGrab"))
			m_iTargetSlotID = weaponSlotFinal.GetWeaponSlotIndex();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		if ( !super.CanBePerformedScript( user ) )
			return false;
		CharacterControllerComponent controller = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		return CanEquipOrReplaceWeapon( user ) && controller.CanPlayItemGesture();
 	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (m_bIsSwappingWeapons)
		{
			string replaceAction = string.Format(WidgetManager.Translate(m_sReplaceActionString), WidgetManager.Translate(m_sWeaponToSwapName));
			outName = string.Format("%1 %2", m_sPickUpActionString, replaceAction);
		}
		else if (m_sWeaponOnGroundType == PRIMARY_WEAPON_TYPE)
			outName = string.Format("%1 %2", m_sPickUpActionString, m_sEquipingSlingString);
		else if (m_sWeaponOnGroundType == SECONDARY_WEAPON_TYPE)
			outName = string.Format("%1 %2", m_sPickUpActionString, m_sEquipingHolsterString);
		else
			outName = m_sPickUpActionString;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override BaseWeaponComponent GetWeaponToSwap( notnull BaseWeaponManagerComponent weaponManager )
	{
		BaseWeaponComponent weaponToSwap;
		WeaponSlotComponent currentWeaponSlot = weaponManager.GetCurrentSlot();
		string weaponSlotType;
		
		foreach ( WeaponSlotComponent weaponSlot: m_aWeaponSlots )
		{
			weaponSlotType = weaponSlot.GetWeaponSlotType();
			
			if ( weaponSlotType != m_sWeaponOnGroundType )
				continue;
			
			// If there is only a single slot compatible with the weapon on the ground it will be used
			if ( m_iSameTypeSlotsCount <= 1 )
			{
				if ( weaponSlot == currentWeaponSlot )
					return null;
				
				weaponToSwap = BaseWeaponComponent.Cast( weaponSlot.GetWeaponEntity().FindComponent( BaseWeaponComponent ) );
				if ( weaponToSwap )
					return weaponToSwap;
			}
			
			// If the slot is not currently equipped it will be used.
			if ( weaponSlot != currentWeaponSlot )
			{
				if ( weaponSlot.GetWeaponEntity() )
				{
					weaponToSwap = BaseWeaponComponent.Cast( weaponSlot.GetWeaponEntity().FindComponent( BaseWeaponComponent ) );
					if ( weaponToSwap )
						return weaponToSwap;
				}
			}
		}
		
		return null;
	}
	
	#endif
};

