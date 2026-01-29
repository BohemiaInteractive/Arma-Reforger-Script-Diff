class SCR_ResourceStorageTypeTitle: BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		EResourceContainerStorageType storageType;
		
		source.Get("m_eStorageType", storageType);
		
		title = SCR_Enum.GetEnumName(EResourceContainerStorageType, storageType);
		string firstLetter = title[0];
		
		title.Replace("_", " ");
		title.ToLower();
		firstLetter.ToUpper();
		
		title = string.Format("%2%3 - %1", source.GetClassName(), firstLetter, title.Substring(1, title.Length() - 1));
		
 		return true;
	}
};

[BaseContainerProps()]
class SCR_ResourceStoragePolicyBase<Class ResourceActorType> : Managed
{
	[Attribute(uiwidget: UIWidgets.Object)]
	protected ref SCR_ResourceContainerStorageQueue<ResourceActorType> m_StorageQueue;
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceContainerStorageQueue<ResourceActorType> GetStorageQueue()
	{
		return m_StorageQueue;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsStorageTypeValid(EResourceContainerStorageType storageType)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetStorageQueue(notnull SCR_ResourceContainerStorageQueue<ResourceActorType> queue)
	{
		m_StorageQueue = queue;
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetStorageQueue()
	{
	#ifdef WORKBENCH
		if (m_StorageQueue)
			m_StorageQueue.Reset();
		else
			Debug.Error2("SCR_ResourceStoragePolicyBase::m_StorageQueue is null.", "A resource container storage queue has to be present.");
	#else
		m_StorageQueue.Reset();
	#endif
	}
}