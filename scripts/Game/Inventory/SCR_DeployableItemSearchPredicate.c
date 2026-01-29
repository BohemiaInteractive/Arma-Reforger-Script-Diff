class SCR_DeployableItemSearchPredicate : InventorySearchPredicate
{
	IEntity m_IgnoredItem;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] ignoredItem
	void SCR_DeployableItemSearchPredicate(IEntity ignoredItem = null)
	{
		m_IgnoredItem = ignoredItem;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		return item != m_IgnoredItem && item.FindComponent(SCR_MultiPartDeployableItemComponent) != null && item.FindComponent(SCR_DeployablePlaceableItemComponent) != null;
	}
}
