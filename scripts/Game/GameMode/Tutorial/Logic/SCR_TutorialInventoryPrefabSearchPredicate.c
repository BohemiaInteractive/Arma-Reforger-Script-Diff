class SCR_TutorialInventoryPrefabSearchPredicate : InventorySearchPredicate
{
    ResourceName m_sSeachPrefab;
	
	//------------------------------------------------------------------------------------------------
    override bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
    { 
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sSeachPrefab))
			return false;
		
		EntityPrefabData prefabData = item.GetPrefabData();
		if (!prefabData)
			return false;
		
        return prefabData.GetPrefabName() == m_sSeachPrefab;
    };
}