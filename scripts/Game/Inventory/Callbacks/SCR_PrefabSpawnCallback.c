class SCR_PrefabSpawnCallback : ScriptedInventoryOperationCallback
{
	protected BaseInventoryStorageComponent m_TargetStorage;

	//------------------------------------------------------------------------------------------------
	override protected void OnComplete()
	{
		SCR_HandSlotStorageComponent handSlotStorage = SCR_HandSlotStorageComponent.Cast(m_TargetStorage);
		if (!handSlotStorage)
			return;

		//since spawning of new entities happens on server then we need to call server version of skip animation to inform the owner that they should not wait for the animation
		handSlotStorage.SkipAnimation_S();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_PrefabSpawnCallback(BaseInventoryStorageComponent targetStorage)
	{
		m_TargetStorage = targetStorage;
	}
}
