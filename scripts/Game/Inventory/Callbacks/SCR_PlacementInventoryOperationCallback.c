void AfterItemPlacementRequestedCallback(notnull SCR_ItemPlacementComponent caller, notnull IEntity item, bool succes, bool equipNext);
typedef func AfterItemPlacementRequestedCallback;
typedef ScriptInvokerBase<AfterItemPlacementRequestedCallback> AfterItemPlacedInvoker;

class SCR_PlacementInventoryOperationCallback : ScriptedInventoryOperationCallback
{
	protected ref AfterItemPlacedInvoker m_AfterPlacementCompleteInvoker;
	protected SCR_ItemPlacementComponent m_ItemPlacementComp;
	protected IEntity m_Item;
	protected bool m_bEquipNext;

	//------------------------------------------------------------------------------------------------
	void SCR_PlacementInventoryOperationCallback(AfterItemPlacedInvoker AfterPlacementCompleteInvoker, SCR_ItemPlacementComponent itemPlacementComp, IEntity item, bool equipNext)
	{
		m_AfterPlacementCompleteInvoker = AfterPlacementCompleteInvoker;
		m_ItemPlacementComp = itemPlacementComp;
		m_Item = item;
		m_bEquipNext = equipNext;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnComplete()
	{
		m_AfterPlacementCompleteInvoker.Invoke(m_ItemPlacementComp, m_Item, true, m_bEquipNext);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnFailed()
	{
		m_AfterPlacementCompleteInvoker.Invoke(m_ItemPlacementComp, m_Item, false, m_bEquipNext);
	}
}