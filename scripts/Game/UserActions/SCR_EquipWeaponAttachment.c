//------------------------------------------------------------------------------------------------
//! Equip weapon attachment
class SCR_EquipWeaponAttachment : SCR_InventoryAction
{
	ChimeraCharacter m_Character;
	GenericEntity m_Owner;
	SCR_InventoryStorageManagerComponent m_InventoryManager;
	InventoryItemComponent m_ItemComponent;

	
	#ifndef DISABLE_INVENTORY
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		manager.EquipWeaponAttachment( pOwnerEntity, pUserEntity );
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if(!m_Owner)
			return false;
		
		if (!m_ItemComponent || m_ItemComponent.GetParentSlot() != null)
			return false;
		
		if(!m_Character)
			m_Character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if(!m_Character)
			return false;
		
		if(!m_InventoryManager)
			m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast(m_Character.FindComponent(SCR_InventoryStorageManagerComponent));
		if(!m_InventoryManager)
			return false;
		
		bool result = m_InventoryManager.CanInsertItem(m_Owner, EStoragePurpose.PURPOSE_ATTACHMENT_PROXY);
		
		return result;
	}	
	#endif
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{		
		if(!m_Owner)
			m_Owner = GenericEntity.Cast(pOwnerEntity);
		if (m_Owner)
		{
			m_ItemComponent = InventoryItemComponent.Cast(m_Owner.FindComponent(InventoryItemComponent));
		}
	}
};