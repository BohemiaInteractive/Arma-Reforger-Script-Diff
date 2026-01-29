class SCR_AIDecideActivity: AITaskScripted
{
	static const string PORT_ACTIVITY_TREE		= "ActivityTree";
	static const string PORT_UPDATE_ACTIVITY	= "UpdateActivity";
	
	SCR_AIGroupUtilityComponent m_UtilityComponent;
	SCR_AIMessageGoal m_GoalMessage;
	SCR_AIMessageInfo m_InfoMessage;

	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		if (owner)
			m_UtilityComponent = SCR_AIGroupUtilityComponent.Cast(owner.FindComponent(SCR_AIGroupUtilityComponent));		
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		bool restartActivity;
		if (!m_UtilityComponent)
		{
			return NodeError(this, owner, "Group utility component is missing.");
		}				
						
		SCR_AIActionBase currentAction = m_UtilityComponent.EvaluateActivity(restartActivity);
		if (!currentAction)
		{
			//Print("AI: Missing behavior tree in " + m_CurrentActivity.ToString(), LogLevel.WARNING);
			return ENodeResult.FAIL;
		}
		else
		{
			if (restartActivity)
			{
				SetVariableOut(PORT_ACTIVITY_TREE, currentAction.m_sBehaviorTree);
				SetVariableOut(PORT_UPDATE_ACTIVITY, restartActivity);
			}
			
			return ENodeResult.SUCCESS;	
		}
		
		return ENodeResult.FAIL;	
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_ACTIVITY_TREE,
		PORT_UPDATE_ACTIVITY
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
};