class SCR_ItemInsertionCallback
{
	IEntity m_Item;
	SCR_InvCallBack m_Callback;
	BaseInventoryStorageComponent m_TargetStorage;
	float m_fExpireTime;

	void SCR_ItemInsertionCallback(notnull IEntity item, SCR_InvCallBack cb, BaseInventoryStorageComponent storage, float time)
	{
		m_Item = item;
		m_Callback = cb;
		m_fExpireTime = time;
		m_TargetStorage = storage;
	}
}