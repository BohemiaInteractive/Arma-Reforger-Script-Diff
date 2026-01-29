class SCR_TutorialInventoryItemSearchPredicate : InventorySearchPredicate
{
    IEntity m_SearchEntity;
	
	//------------------------------------------------------------------------------------------------
    override bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
    { 
        return m_SearchEntity && m_SearchEntity == item;
    };
}