class SCR_SortableItem<Class ActualType>
{
	[SortAttribute()]
	int m_iSortBy = 0;

	ActualType m_Item;
	
	//------------------------------------------------------------------------------------------------
	void SCR_SortableItem(const ActualType item, const int sortValue)
	{
		m_Item = item;
		m_iSortBy = sortValue;
	}
}
