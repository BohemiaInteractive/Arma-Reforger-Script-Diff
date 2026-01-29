[BaseContainerProps()]
class SCR_MemoryNetworkedStat : SCR_BaseNetworkedStat
{
	// Last checked local RAM.
	protected int m_iLocalMemory;

	// Last checked authority RAM.
	protected int m_iAuthorityMemory;

	//------------------------------------------------------------------------------------------------
	int GetLastCheckedLocalMemory()
	{
		if (m_iLocalMemory == 0)
			return 0;

		return m_iLocalMemory / 1024; // Division by 1024 for KB to MB conversion.
	}

	//------------------------------------------------------------------------------------------------
	int GetLastCheckedAuthorityMemory()
	{
		if (m_iAuthorityMemory == 0)
			return 0;

		return m_iAuthorityMemory / 1024; // Division by 1024 for KB to MB conversion.
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateStat()
	{
		int localMemory = System.MemoryAllocationKB();
		if (localMemory == m_iLocalMemory)
			return;

		m_iLocalMemory = localMemory;

		RplComponent rplComponent = m_Owner.GetRplComponent();
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
		{
			m_iAuthorityMemory = m_iLocalMemory;
			m_Owner.SetAuthorityStatValue(m_eStatType, m_iAuthorityMemory);
		}

		InvokeStatValueChange();
	}

	//------------------------------------------------------------------------------------------------
	override void OnAuthorityStatValueSet(int value)
	{
		m_iAuthorityMemory = value;
		InvokeStatValueChange();
	}
}
