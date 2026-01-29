class SCR_AIGetCombatMovementParameters : AITaskScripted
{
	protected static const string PORT_STANCE = "Stance";
	protected static const string PORT_MOVEMENT_TYPE = "MovementType";
	
	protected SCR_AIUtilityComponent m_Utility;
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_STANCE,
		PORT_MOVEMENT_TYPE
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	static override bool VisibleInPalette()
	{
		return true;
	}
	
	static override string GetOnHoverDescription()
	{
		return "Returns stance and movement type while advancing towards enemy";
	}
	
	override void OnInit(AIAgent owner)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (!m_Utility)
			NodeError(this, owner, "Utility component not found");
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		
		if (!m_Utility)
			return ENodeResult.FAIL;
		
		EAIThreatState threat = m_Utility.m_ThreatSystem.GetState();
		ECharacterStance stance = ECharacterStance.STAND;
		EMovementType moveType = EMovementType.RUN;
		
		switch (threat)
		{
			case EAIThreatState.VIGILANT:
			case EAIThreatState.SAFE:
			{
				stance = ECharacterStance.STAND;
				moveType = EMovementType.RUN; // SPRINT Temporary disabled because sometimes soldiers don't sprint but walk instead
				break;
			}
				
			case EAIThreatState.ALERTED:			
			case EAIThreatState.THREATENED:
			{
				stance = ECharacterStance.CROUCH;
				moveType = EMovementType.RUN;
				break;
			}
		}
		
		SetVariableOut(PORT_STANCE, stance);
		SetVariableOut(PORT_MOVEMENT_TYPE, moveType);
		return ENodeResult.SUCCESS;
	}
};
