// Script File
class SCR_AIDropWeapon: AITaskScripted
{
	private GenericEntity m_Controlled = null;
	private BaseWeaponManagerComponent m_WpnManager = null;
	private SCR_CharacterControllerComponent m_Controller = null;
	private SCR_InventoryStorageManagerComponent m_Inventory;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_Controlled = null;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{				
		IEntity contr = owner.GetControlledEntity();
		if (m_Controlled != contr)
		{
			m_Controlled = GenericEntity.Cast(contr);
			if (m_Controlled)
			{
				m_WpnManager = BaseWeaponManagerComponent.Cast(m_Controlled.FindComponent(BaseWeaponManagerComponent));
				m_Controller = SCR_CharacterControllerComponent.Cast(m_Controlled.FindComponent(SCR_CharacterControllerComponent));
				m_Inventory = SCR_InventoryStorageManagerComponent.Cast(m_Controlled.FindComponent(SCR_InventoryStorageManagerComponent));
			}
		}
		
		if (!m_Controlled || !m_WpnManager || !m_Controller || !m_Inventory)
			return NodeError(this, owner, "Missing components.");
			
		WeaponSlotComponent slot = m_WpnManager.GetCurrentSlot();
		if (!slot)
			return ENodeResult.FAIL;
		IEntity weaponEntity = slot.GetWeaponEntity();
		if (!weaponEntity)
			return ENodeResult.FAIL;
		InventoryItemComponent pInvComp = InventoryItemComponent.Cast( weaponEntity.FindComponent( InventoryItemComponent ) );
		if (!pInvComp)
			return ENodeResult.FAIL;
		InventoryStorageSlot parentSlot = pInvComp.GetParentSlot();
		if (!parentSlot)
			return ENodeResult.FAIL;
		bool removed = m_Inventory.TryRemoveItemFromStorage(weaponEntity, parentSlot.GetStorage());
		if (removed)
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;		
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}

};