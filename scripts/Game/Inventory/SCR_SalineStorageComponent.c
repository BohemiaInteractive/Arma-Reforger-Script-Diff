class SCR_SalineStorageComponentClass : SCR_EquipmentStorageComponentClass
{
}

class SCR_SalineMovedCallback : ScriptedInventoryOperationCallback
{
	SCR_CharacterInventoryStorageComponent m_CharInventoryStorageComp;
	IEntity m_SalineBag;
	float m_fItemRegenerationDuration;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnComplete()
	{
		m_CharInventoryStorageComp.RemoveItemFromQuickSlot(m_SalineBag);
		//Destroy saline bag when the healing effect wears off
		GetGame().GetCallqueue().CallLater(DestroySalineBag, m_fItemRegenerationDuration * 1000, false, m_SalineBag);
	}

	//------------------------------------------------------------------------------------------------
	protected void DestroySalineBag(IEntity item)
	{
		if (!item)
			return;
		
		RplComponent.DeleteRplEntity(item, false);
	}
}

class SCR_SalineStorageComponent : SCR_EquipmentStorageComponent
{
	ref SCR_SalineMovedCallback m_SalineMovedCallback = new SCR_SalineMovedCallback();
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] target
	//! \param[in] eHitZoneGroup
	//! \param[in] salineBag
	//! \param[in] itemRegenerationDuration
	//! \return
	bool AddSalineBagToSlot(IEntity target, ECharacterHitZoneGroup eHitZoneGroup, IEntity salineBag, float itemRegenerationDuration)
	{
		if (!target || !salineBag)
			return false;

		RplComponent rplComp = RplComponent.Cast(target.FindComponent(RplComponent));
		if (rplComp && rplComp.IsProxy())
			return false;

		SCR_SalineStorageComponent SalineStorageComp = SCR_SalineStorageComponent.Cast(target.FindComponent(SCR_SalineStorageComponent));
		if (!SalineStorageComp)
			return false;
		
		SCR_InventoryStorageManagerComponent storageMan = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageMan)
			return false;
		
		SCR_SalineBagStorageSlot salineTargetSlot;
		for (int i, count = SalineStorageComp.GetSlotsCount(); i < count; i++)
		{
			salineTargetSlot = SCR_SalineBagStorageSlot.Cast(SalineStorageComp.GetSlot(i));
			if (!salineTargetSlot)
				continue;

			if (salineTargetSlot.GetAssociatedHZGroup() != eHitZoneGroup)
				continue;
			
			if (salineTargetSlot.GetItem(i) == salineBag)
				return false;
			
			if (salineTargetSlot.GetItem(i))
			{
				Debug.Error("salineBagSlot already contained some item");
				return false;
			}

			break;
		}
		
		if (!salineTargetSlot)
			return false;
		
		SCR_CharacterInventoryStorageComponent charInventoryStorageComp = SCR_CharacterInventoryStorageComponent.Cast(GetOwner().FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!charInventoryStorageComp)
			return false;

		m_SalineMovedCallback.m_SalineBag = salineBag;
		m_SalineMovedCallback.m_CharInventoryStorageComp = charInventoryStorageComp;
		m_SalineMovedCallback.m_fItemRegenerationDuration = itemRegenerationDuration;
 		
		if (storageMan.TryMoveItemToStorage(salineBag, SalineStorageComp, salineTargetSlot.GetID(), m_SalineMovedCallback))
			return true;
		
		InventoryItemComponent invComp = InventoryItemComponent.Cast(salineBag.FindComponent(InventoryItemComponent));
		if (!invComp)
			return false;
		
		if (!invComp.GetParentSlot())
		{
			if (storageMan.TryInsertItemInStorage(salineBag, SalineStorageComp, salineTargetSlot.GetID(), m_SalineMovedCallback))
				return true;
		}
		else
		{
			if (storageMan.TryMoveItemToStorage(salineBag, SalineStorageComp, salineTargetSlot.GetID(), m_SalineMovedCallback))
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

		SCR_SalineBagStorageSlot salineSlot = SCR_SalineBagStorageSlot.Cast(GetSlot(slotID));
		if (!salineSlot)
			return;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;

		damageMgr.SetSalineBaggedGroup(salineSlot.GetAssociatedHZGroup(), true);		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnRemovedFromSlot(IEntity item, int slotID)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return;
		
		SCR_SalineBagStorageSlot salineSlot = SCR_SalineBagStorageSlot.Cast(GetSlot(slotID));
		if (!salineSlot)
			return;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;

		damageMgr.SetSalineBaggedGroup(salineSlot.GetAssociatedHZGroup(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool ShouldPreviewAttachedItems()
	{
		return true;
	}
}
