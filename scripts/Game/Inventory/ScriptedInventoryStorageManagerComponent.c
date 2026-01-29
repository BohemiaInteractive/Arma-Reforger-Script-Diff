class ScriptedInventoryStorageManagerComponentClass : InventoryStorageManagerComponentClass
{
}

class ScriptedInventoryStorageManagerComponent : InventoryStorageManagerComponent
{
	ref ScriptInvokerBase<ScriptInvokerEntityAndStorageMethod> m_OnItemAddedInvoker = new ScriptInvokerBase<ScriptInvokerEntityAndStorageMethod>();
	ref ScriptInvokerBase<ScriptInvokerEntityAndStorageMethod> m_OnItemRemovedInvoker = new ScriptInvokerBase<ScriptInvokerEntityAndStorageMethod>();

	//------------------------------------------------------------------------------------------------
	override protected void OnItemAdded(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		super.OnItemAdded(storageOwner, item);
		
		m_OnItemAddedInvoker.Invoke(item, storageOwner);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnItemRemoved(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		super.OnItemRemoved(storageOwner, item);
		
		m_OnItemRemovedInvoker.Invoke(item, storageOwner);
	}
}
