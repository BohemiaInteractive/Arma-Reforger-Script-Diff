//------------------------------------------------------------------------------------------------
//! modded version for to be used with the inventory 2.0 
class SCR_EquipGadgetAction: SCR_InventoryAction
{
	[Attribute("#AR-Inventory_NoSlot", desc: "Invalid reason no free slot")]
	protected LocalizedString m_sInvalid_NoFreeSlot;

	#ifndef DISABLE_INVENTORY
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;

		SCR_InventoryStorageManagerComponent inventoryStorageManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (!inventoryStorageManager)
			return false;

		SCR_HandSlotStorageComponent storageComp = SCR_HandSlotStorageComponent.Cast(inventoryStorageManager.FindStorageForItem(GetOwner(), EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT));
		if (!storageComp)
		{
			switch (inventoryStorageManager.GetReturnCode())
			{
				case EInventoryRetCode.RETCODE_ITEM_TOO_BIG:
				{
					SetCannotPerformReason(SREASON_TOO_BIG);
					break;
				}

				case EInventoryRetCode.RETCODE_ITEM_TOO_BIG:
				{
					SetCannotPerformReason(SREASON_TOO_HEAVY);
					break;
				}

				default:
				{
					SetCannotPerformReason(m_sInvalid_NoFreeSlot);
				}
			}
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (CanBePerformedScript(pUserEntity))
			manager.EquipGadget( pOwnerEntity );
	}
	#endif
};