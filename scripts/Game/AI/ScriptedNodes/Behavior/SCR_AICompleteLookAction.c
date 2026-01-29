class SCR_AICompleteLookAction : AITaskScripted
{
	SCR_AIUtilityComponent m_Utility;

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Utility)
		{
			m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
			if (!m_Utility)
				return ENodeResult.FAIL;
		}
		
		m_Utility.m_LookAction.Complete();
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------	
	protected static override bool VisibleInPalette() {return true;}	
};