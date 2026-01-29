class SCR_UserActionWithOccupancy: ScriptedUserAction
{
	protected IEntity m_Occupant;
	
	//------------------------------------------------------------------------------------------------
	bool IsOccupied ()
	{
		return m_Occupant != null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if (!m_Occupant)
		{
			m_Occupant = pUserEntity;
			StartAction(pUserEntity);
		}
		else
		{
			m_Occupant = null;
			StopAction(pUserEntity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StartAction(IEntity pUserEntity);
	
	//------------------------------------------------------------------------------------------------
	void StopAction(IEntity pUserEntity);
}