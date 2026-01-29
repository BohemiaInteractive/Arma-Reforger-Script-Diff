[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "")]
class SCR_MineWeaponComponentClass : WeaponComponentClass
{
}

class SCR_MineWeaponComponent : WeaponComponent
{
	[RplProp(onRplName: "OnFlagEntitySync")]
	protected RplId m_FlagEntityId = RplId.Invalid();

	protected IEntity m_FlagEntity;

	//------------------------------------------------------------------------------------------------
	bool IsFlagged()
	{
		return m_FlagEntity != null;
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetFlagEntity()
	{
		return m_FlagEntity;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFlagParentSlotChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(m_FlagEntity.FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.m_OnParentSlotChangedInvoker.Remove(OnFlagParentSlotChanged);

		m_FlagEntity = null;
		m_FlagEntityId = RplId.Invalid();
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Call only on server!
	void SetFlag(IEntity flag)
	{
		if (m_FlagEntity || !flag)
			return; // This should never happen!

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(flag.FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.m_OnParentSlotChangedInvoker.Insert(OnFlagParentSlotChanged);

		m_FlagEntity = flag;

		RplComponent rpl = RplComponent.Cast(flag.FindComponent(RplComponent));
		if (rpl)
			m_FlagEntityId = rpl.Id();

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFlagEntitySync()
	{
		const RplComponent rpl = RplComponent.Cast(Replication.FindItem(m_FlagEntityId));
		if (rpl)
			m_FlagEntity = rpl.GetEntity();
		else
			m_FlagEntity = null;
	}
}
