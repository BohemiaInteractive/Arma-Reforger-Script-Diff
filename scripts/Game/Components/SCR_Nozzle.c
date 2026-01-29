class SCR_Nozzle
{
	private int m_iProviderIndex = -1;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] iProviderIndex
	void SetProvider(int iProviderIndex)
	{
		m_iProviderIndex = iProviderIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetProvider()
	{
		return m_iProviderIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] iProviderIndex
	void SCR_Nozzle(int iProviderIndex)
	{
		SetProvider(iProviderIndex);
	}
}
