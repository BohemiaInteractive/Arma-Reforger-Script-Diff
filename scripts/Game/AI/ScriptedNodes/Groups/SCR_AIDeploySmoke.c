class SCR_AIDeploySmokeCover : AITaskScripted
{	
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	
	static const int MAX_SMOKE_POSITION_COUNT = 3; // Max number of smoke grenades that can be thrown at one time to create smoke cover wall
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (group)
			m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_GroupUtilityComponent || !m_GroupUtilityComponent.m_aInfoComponents)
			return ENodeResult.FAIL;
		
		SCR_AIActivityBase currentActivity = SCR_AIActivityBase.Cast(m_GroupUtilityComponent.GetCurrentAction());
		if (!currentActivity)
			return ENodeResult.FAIL;
		
		// Find smoke cover feature in current activity
		SCR_AIActivitySmokeCoverFeature smokeCoverFeature = SCR_AIActivitySmokeCoverFeature.Cast(
			currentActivity.FindActivityFeature(SCR_AIActivitySmokeCoverFeature));
		
		// Exit if smoke cover feature is not supported in current activity
		if (!smokeCoverFeature)
			return ENodeResult.FAIL;
		
		if (smokeCoverFeature.ExecuteForActivity(currentActivity, MAX_SMOKE_POSITION_COUNT))
			return ENodeResult.SUCCESS;
		
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	static override protected bool CanReturnRunning() { return false; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Getter returns group member available for specific role";
	}	
}