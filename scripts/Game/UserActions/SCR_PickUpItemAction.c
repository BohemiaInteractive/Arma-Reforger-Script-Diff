//------------------------------------------------------------------------------------------------
class SCR_PickUpItemAction : SCR_InventoryAction
{
	#ifndef DISABLE_INVENTORY
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		manager.InsertItem(pOwnerEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;

		auto pInventoryManager = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!pInventoryManager)
			return false;

		pInventoryManager.SetReturnCodeDefault();

		if (!pInventoryManager.IsAnimationReady() || !pInventoryManager.CanInsertItem(m_Item.GetOwner(), EStoragePurpose.PURPOSE_DEPOSIT))
		{
			SetCannotPerformReason(GetReason(pInventoryManager));
			return false;
		}

		return true;
	}
	#endif
}
