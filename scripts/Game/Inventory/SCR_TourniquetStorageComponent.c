class SCR_TourniquetStorageComponentClass : SCR_EquipmentStorageComponentClass
{
}

class SCR_TourniquetMovedCallback : ScriptedInventoryOperationCallback
{
	SCR_CharacterInventoryStorageComponent m_CharInventoryStorageComp;
	IEntity m_Tourniquet;
	int m_iSlotId;
	BaseInventoryStorageComponent m_Storage;
	bool m_bRemove;
	ref ScriptInvoker<int> m_OnTourniquetMoved;

	//------------------------------------------------------------------------------------------------
	override protected void OnComplete()
	{
		if (m_bRemove)
			m_CharInventoryStorageComp.RemoveItemFromQuickSlot(m_Tourniquet);

		SCR_InventoryMenuUI menu = SCR_InventoryMenuUI.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.Inventory20Menu));
		if (menu)
		{
			if (m_Tourniquet)
			{
				InventoryItemComponent itemComp = InventoryItemComponent.Cast(m_Tourniquet.FindComponent(InventoryItemComponent));
				if (itemComp && itemComp.GetParentSlot())
				{
					BaseInventoryStorageComponent tqStorage = itemComp.GetParentSlot().GetStorage();
					if (tqStorage)
					{
						SCR_InventoryStorageBaseUI storageUI = menu.GetStorageUIByBaseStorageComponent(tqStorage);
						if (storageUI)
							storageUI.Refresh();
					}
				}
			}

			if (m_Storage)
			{
				SCR_InventoryStorageBaseUI storageUI = menu.GetStorageUIByBaseStorageComponent(m_Storage);
				if (storageUI)
					storageUI.Refresh();
			}	
		}
		
		if (m_OnTourniquetMoved)
			m_OnTourniquetMoved.Invoke(m_iSlotId);
	}
}

class SCR_TourniquetStorageComponent : SCR_EquipmentStorageComponent
{
	protected ref SCR_TourniquetMovedCallback m_TourniquetMovedCallback = new SCR_TourniquetMovedCallback();
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] target
	//! \param[in] eHitZoneGroup
	//! \param[in] tourniquet
	void AddTourniquetToSlot(IEntity target, ECharacterHitZoneGroup eHitZoneGroup, IEntity tourniquet)
	{
		SCR_TourniquetStorageComponent tourniquetStorageComp = SCR_TourniquetStorageComponent.Cast(target.FindComponent(SCR_TourniquetStorageComponent));
		if (!tourniquetStorageComp)
			return;		
		
		SCR_InventoryStorageManagerComponent storageMan = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageMan)
			return;
	
		if (!tourniquet)
			return;
		
		SCR_TourniquetStorageSlot tqTargetSlot;
		for (int i, count = tourniquetStorageComp.GetSlotsCount(); i < count; i++)
		{
			tqTargetSlot = SCR_TourniquetStorageSlot.Cast(tourniquetStorageComp.GetSlot(i));
			if (!tqTargetSlot)
				continue;

			if (tqTargetSlot.GetAssociatedHZGroup() != eHitZoneGroup)
				continue;
			
			//Return if slot already contains this particular tourniquet. Likely function got called twice harmlessly
			if (tqTargetSlot.GetItem(i) == tourniquet)
				return;		

			if (tqTargetSlot.GetItem(i))
			{
				Print("TourniquetSlot already contained another item", LogLevel.WARNING);
				return;
			}

			break;
		}
		
		SCR_CharacterInventoryStorageComponent charInventoryStorageComp = SCR_CharacterInventoryStorageComponent.Cast(GetOwner().FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!charInventoryStorageComp)
			return;

		m_TourniquetMovedCallback.m_Tourniquet = tourniquet;
		m_TourniquetMovedCallback.m_CharInventoryStorageComp = charInventoryStorageComp;
		m_TourniquetMovedCallback.m_iSlotId = eHitZoneGroup;

		InventoryItemComponent itemComp = InventoryItemComponent.Cast(tourniquet.FindComponent(InventoryItemComponent));
		if (itemComp.GetParentSlot())
			m_TourniquetMovedCallback.m_Storage = itemComp.GetParentSlot().GetStorage();
		else
			m_TourniquetMovedCallback.m_Storage = null;
		
		m_TourniquetMovedCallback.m_bRemove = true;
		
		if (storageMan.TryMoveItemToStorage(tourniquet, tourniquetStorageComp, tqTargetSlot.GetID(), m_TourniquetMovedCallback))
			return;
		
		InventoryItemComponent invComp = InventoryItemComponent.Cast(tourniquet.FindComponent(InventoryItemComponent));
		if (!invComp)
			return;
		
		if (!invComp.GetParentSlot())
		{
			if (storageMan.TryInsertItemInStorage(tourniquet, tourniquetStorageComp, tqTargetSlot.GetID(), m_TourniquetMovedCallback))
				return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] eHitZoneGroup
	//! \param[in] retrievingCharacter
	//! \return
	bool RemoveTourniquetFromSlot(ECharacterHitZoneGroup eHitZoneGroup, IEntity retrievingCharacter = null)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!damageMgr)
			return false;
		
		SCR_TourniquetStorageComponent tourniquetStorageComp = SCR_TourniquetStorageComponent.Cast(GetOwner().FindComponent(SCR_TourniquetStorageComponent));
		if (!tourniquetStorageComp)
			return false;

		if (!retrievingCharacter)
			retrievingCharacter = GetOwner();
	
		SCR_InventoryStorageManagerComponent storageMan = SCR_InventoryStorageManagerComponent.Cast(retrievingCharacter.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageMan)
			return false;
				
		IEntity targetTourniquet;
		SCR_TourniquetStorageSlot tqTargetSlot;
		for (int i, count = tourniquetStorageComp.GetSlotsCount(); i < count; i++)
		{
			tqTargetSlot = SCR_TourniquetStorageSlot.Cast(tourniquetStorageComp.GetSlot(i));
			if (!tqTargetSlot)
				continue;

			if (tqTargetSlot.GetAssociatedHZGroup() != eHitZoneGroup)
				continue;
			
			targetTourniquet = tqTargetSlot.GetItem(i);
			break;
		}

		BaseInventoryStorageComponent targetStorage = storageMan.FindStorageForItem(targetTourniquet, EStoragePurpose.PURPOSE_DEPOSIT);
		m_TourniquetMovedCallback.m_Storage = targetStorage;
		m_TourniquetMovedCallback.m_bRemove = false;

		if (!targetTourniquet)
			return false;
		
		bool success = false;
		
		if (storageMan.TryMoveItemToStorage(targetTourniquet, targetStorage, tqTargetSlot.GetID(), m_TourniquetMovedCallback))
			success = true;
		
		InventoryItemComponent invComp = InventoryItemComponent.Cast(targetTourniquet.FindComponent(InventoryItemComponent));
		if (!invComp)
			return false;
		
		if (!success && !invComp.GetParentSlot())
		{
			if (storageMan.TryInsertItemInStorage(targetTourniquet, targetStorage, tqTargetSlot.GetID(), m_TourniquetMovedCallback))
				success = true;
		}
		else if (!success)
		{
			if (storageMan.TryMoveItemToStorage(targetTourniquet, targetStorage, tqTargetSlot.GetID(), m_TourniquetMovedCallback))
				success = true;
		}
		
		if (success)
		{
			SCR_HitZone hitZone;
			array<ref SCR_PersistentDamageEffect> tourniquetEffects = damageMgr.GetAllPersistentEffectsOfType(SCR_TourniquetDamageEffect);
			foreach (SCR_PersistentDamageEffect effect : tourniquetEffects)
			{
				hitZone = SCR_HitZone.Cast(effect.GetAffectedHitZone());
				if (!hitZone)
					continue;
				
				if (tqTargetSlot.GetAssociatedHZGroup() == hitZone.GetHitZoneGroup())
					damageMgr.TerminateDamageEffect(effect);
			}
			
			return true;
		}
		
		return false;	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAddedToSlot(IEntity item, int slotID)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return;

		SCR_TourniquetStorageSlot tqSlot = SCR_TourniquetStorageSlot.Cast(GetSlot(slotID));
		if (!tqSlot)
			return;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;

		damageMgr.SetTourniquettedGroup(tqSlot.GetAssociatedHZGroup(), true);		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnRemovedFromSlot(IEntity item, int slotID)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return;
		
		SCR_TourniquetStorageSlot tqSlot = SCR_TourniquetStorageSlot.Cast(GetSlot(slotID));
		if (!tqSlot)
			return;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;

		damageMgr.SetTourniquettedGroup(tqSlot.GetAssociatedHZGroup(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnTourniquetMoved()
	{
		if (!m_TourniquetMovedCallback.m_OnTourniquetMoved)
			m_TourniquetMovedCallback.m_OnTourniquetMoved = new ScriptInvoker();

		return m_TourniquetMovedCallback.m_OnTourniquetMoved;
	}
}
