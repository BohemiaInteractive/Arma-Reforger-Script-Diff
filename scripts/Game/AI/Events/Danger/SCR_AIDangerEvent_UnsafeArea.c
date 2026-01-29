class SCR_AIDangerEvent_UnsafeArea : AIDangerEvent
{
	protected float m_fRadius;
	
	protected EMovementType m_eMovementType = EMovementType.RUN;
	
	//------------------------------------------------------------------------------------------------
	void SetRadius(float radius)
	{
		m_fRadius = radius;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetRadius()
	{
		return m_fRadius;
	}
}