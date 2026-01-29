class SCR_AIGetAnimationScriptParameters : AITaskScripted
{
	static const string PORT_ROOT_ENTITY			= "RootEntity";
	static const string PORT_POSITION 				= "AnimationPosition";
	static const string PORT_DURATION 				= "AnimationDuration";
	static const string PORT_DIRECTION				= "AnimationDirection";
	static const string PORT_ANIMATION_INDEX		= "AnimationIndex";
	static const string PORT_AGENT_SCRIPT 			= "AgentScript";
	
	protected int m_iAnimationIndex;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_iAnimationIndex = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIAnimationScript agentScript;
		IEntity rootEntity;
		if (!GetVariableIn(PORT_AGENT_SCRIPT, agentScript) || !agentScript)
			return NodeError(this, owner, "Missing script reference for this agent!");
		if (!GetVariableIn(PORT_ROOT_ENTITY, rootEntity) || !rootEntity)
			return NodeError(this, owner, "Missing root entity to find world position for animation!");
		
		if (!agentScript.IsAnimationIndexValid(m_iAnimationIndex))
			return ENodeResult.FAIL;
		
		float duration = agentScript.GetAnimationDuration(m_iAnimationIndex);
		if (duration < 0)
			duration = int.MAX;
		
		SetVariableOut(PORT_POSITION, agentScript.GetAnimationPosition(rootEntity, m_iAnimationIndex));
		SetVariableOut(PORT_DURATION, duration);
		
		vector mat[4];
		agentScript.GetAnimationWorldTransform(rootEntity, m_iAnimationIndex, mat);
		vector directionVector = vector.Forward.Multiply3(mat) + mat[3];
		
		SetVariableOut(PORT_DIRECTION, directionVector);
		SetVariableOut(PORT_ANIMATION_INDEX, m_iAnimationIndex);
		
		m_iAnimationIndex ++;
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette() { return true; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_POSITION,
		PORT_DURATION,
		PORT_DIRECTION,
		PORT_ANIMATION_INDEX,
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_AGENT_SCRIPT,
		PORT_ROOT_ENTITY
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "GetAnimationScriptParameters: Gets position, direction of FORWARD vector and duration of provided AIAnimationScript";
	}
};
