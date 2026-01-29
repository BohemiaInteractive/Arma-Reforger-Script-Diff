enum EResourceReason
{
	SUFFICIENT,		// Sufficient Resource Available
	INSUFICIENT,	// Insufficient Resource Available
	UNAVAILABLE		// No Resource Available
}

// TODO: rename Consumtion -> Consumption
class SCR_ResourceConsumtionResponse
{
	protected float				m_fAvailableResource;
	protected float				m_fResourceMultiplier;
	protected float				m_fRange;
	protected EResourceReason	m_eReason;
	
	//------------------------------------------------------------------------------------------------
	void SCR_ResourceConsumtionResponse(float availableResource = 0, float resourceMultiplier = 0, float range = 0, EResourceReason reasonCode = EResourceReason.UNAVAILABLE)
	{
		m_fAvailableResource	= availableResource;
		m_fResourceMultiplier	= resourceMultiplier;
		m_fRange			= range;
		m_eReason			= reasonCode;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetAvailableSupply()
	{
		return m_fAvailableResource;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetResourceMultiplier()
	{
		return m_fResourceMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetRange()
	{
		return m_fRange;
	}
	
	//------------------------------------------------------------------------------------------------
	EResourceReason GetReason()
	{
		return m_eReason;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAvailableResource(float availableResource)
	{
		m_fAvailableResource = availableResource;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetResourceMultiplier(float resourceMultiplier)
	{
		m_fResourceMultiplier = resourceMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRange(float range)
	{
		m_fRange = range;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetReason(EResourceReason reasonCode)
	{
		m_eReason = reasonCode;
	}
}

// //! classname typo fix
// class SCR_ResourceConsumptionResponse : SCR_ResourceConsumtionResponse
// {
// }
