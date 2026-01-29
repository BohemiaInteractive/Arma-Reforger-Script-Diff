class SCR_AIDecoTestIsAvailable : DecoratorTestScripted
{
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{	
		if (controlled)
		{	
			AIControlComponent aiContr = AIControlComponent.Cast(controlled.FindComponent(AIControlComponent));
			if (!aiContr)
				return false;
			SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(aiContr.GetAIAgent());
			if (!chimeraAgent)
				return false;
			auto  infoComponent = chimeraAgent.m_InfoComponent;
			if (!infoComponent)
			{
				Debug.Error("Missing AIInfoComponent in for IEntity: " + controlled.ToString());
				return false;
			}
			return infoComponent.GetAIState() == EUnitAIState.AVAILABLE;
		}
		return false;	
	}
};