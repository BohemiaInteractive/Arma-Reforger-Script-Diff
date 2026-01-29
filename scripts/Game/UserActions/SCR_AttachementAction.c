enum EWeaponAttachmentActionType
{
	ScopeDetach,
	Magazine,
	UGL,
	Muzzle
}

class SCR_AttachementAction : SCR_InventoryAction
{
	#ifndef DISABLE_INVENTORY
    [Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(EWeaponAttachmentActionType) )]
	EWeaponAttachmentActionType m_WeaponAttachmentActionType;	
	
	SCR_InventoryStorageManagerComponent m_InventoryManager;
	IEntity attachment;
	InventoryItemComponent m_InventoryItemComp;
    
	//------------------------------------------------------------------------------------------------
    override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_InventoryItemComp = InventoryItemComponent.Cast(pOwnerEntity.FindComponent(InventoryItemComponent));
	}

	//------------------------------------------------------------------------------------------------
    override bool CanBeShownScript(IEntity user)
    {
		if(!m_InventoryItemComp || !m_InventoryItemComp.GetParentSlot())
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if(!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();	
		if(!controller)
			return false;		
				
		if(!controller.GetInspect())
			return false;
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if(!weaponManager)
			return false;
		
		if(!m_InventoryManager)
			m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if(!m_InventoryManager)
			return false;

		BaseWeaponComponent weaponComp = controller.GetWeaponManagerComponent().GetCurrentWeapon();
		if(!weaponComp || weaponComp.GetOwner() != m_InventoryItemComp.GetParentSlot().GetOwner())
			return false;	
		
		attachment = m_InventoryItemComp.GetOwner();
		
		return (attachment != null);
    }

	//------------------------------------------------------------------------------------------------
    override bool CanBePerformedScript(IEntity user)
    {
		if(!m_InventoryManager)
			return false;
		BaseInventoryStorageComponent storage = m_InventoryManager.FindStorageForItem(attachment);
		if (!storage)
			return false;
        return m_InventoryManager.CanMoveItemToStorage(attachment, storage);
    }

	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		BaseInventoryStorageComponent storage = m_InventoryManager.FindStorageForItem(attachment, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT);
		
		if (!storage)
			storage = m_InventoryManager.FindStorageForItem(attachment);
		
		manager.TryMoveItemToStorage(attachment, storage);
		manager.PlayItemSound(pOwnerEntity, "SOUND_UNEQUIP");
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_InventoryItemComp)
			return false;
		UIInfo actionInfo = GetUIInfo();
		UIInfo itemInfo = m_InventoryItemComp.GetUIInfo();
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
	#endif
}