class SCR_AnalyticsPlayerSupplySpendingData
{
	protected ref map<string, int> m_mItemSupplyCosts = new map<string, int>();

	//------------------------------------------------------------------------------------------------
	int GetAggregateSupplyCost()
	{
		int aggregateSupplyCost, i;
		for (i; i < m_mItemSupplyCosts.Count(); i++)
		{
			aggregateSupplyCost += m_mItemSupplyCosts.GetElement(i);
		}

		return aggregateSupplyCost;
	}

	//------------------------------------------------------------------------------------------------
	map<string, int> GetItemSupplyCosts()
	{
		return m_mItemSupplyCosts;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds new item type and it's cost to the list
	//! If item is already in the list, adds current cost to the already saved cost
	//! \param[in] itemName
	//! \param[in] cost
	void AddItemSupplyCost(string itemName, int cost)
	{
		if (cost == 0)
			return;

		m_mItemSupplyCosts.Set(itemName, m_mItemSupplyCosts.Get(itemName) + cost);
	}
}
