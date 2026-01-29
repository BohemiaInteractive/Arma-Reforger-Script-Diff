//------------------------------------------------------------------------------------------------
//! modded version for to be used with the inventory 2.0 
class SCR_EquipItemAction: SCR_InventoryAction
{
	[Attribute("-1", desc: "If this value is different thatn -1 then script will ignore slot that is normally used fot this\ntype of an item and it will try to use the slot with provided id.\nFirst slot number is 0 while last is 9", params: "-1 9 1")]
	protected int m_iEquipmentSlotIdOverride;

	protected bool m_bShowBindingInfo;

	protected const LocalizedString REASON_NO_SPACE = "#AR-SupportStation_Resupply_ActionInvalid_NoInventorySpace";
	protected const LocalizedString TEXT_SLOT_ID = "#AR-ControlsHint_BindToQuickslot";
	protected const string FORMAT = "%1 (%2)";

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;

		SCR_InventoryStorageManagerComponent pInventoryManager = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!pInventoryManager)
			return false;

		if (!pInventoryManager.CanInsertItem(GetOwner()))
		{
			m_bShowBindingInfo = false;
			SetCannotPerformReason(REASON_NO_SPACE);
			return false;
		}

		if (m_iEquipmentSlotIdOverride < 0)
			return true;

		SCR_CharacterInventoryStorageComponent characterStorage = pInventoryManager.GetCharacterStorage();
		if (!characterStorage)
			return false;

		IEntity boundItem = characterStorage.GetItemFromQuickSlot(m_iEquipmentSlotIdOverride);
		if (boundItem && boundItem.GetPrefabData().GetPrefabName() == GetOwner().GetPrefabData().GetPrefabName())
			m_bShowBindingInfo = false;
		else
			m_bShowBindingInfo = true;

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_InvEquipAnyItemCB inventoryCallBack = new SCR_InvEquipAnyItemCB();
		inventoryCallBack.m_pItem = pOwnerEntity;
		inventoryCallBack.m_pStorageToPickUp = manager.GetCharacterStorage();
		inventoryCallBack.m_iSlotToFocus = m_iEquipmentSlotIdOverride;
		inventoryCallBack.m_bShouldEquip = true;

		manager.InsertItem(pOwnerEntity, cb: inventoryCallBack);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_bShowBindingInfo)
			return false;

		UIInfo info = GetUIInfo();
		if (!info)
			return false;

		outName = info.GetName();
		if (m_iEquipmentSlotIdOverride < 0)
			return true;

		outName = string.Format(FORMAT, outName, WidgetManager.Translate(TEXT_SLOT_ID, m_iEquipmentSlotIdOverride + 1));
		return true;
	}
}
