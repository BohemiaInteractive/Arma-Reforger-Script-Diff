class SCR_AIGetGestureParameters : AITaskScripted
{
	static const string PORT_GESTURE_TYPE			= "eGestureType";

	static const string PORT_DURATION 				= "AnimationDuration";
	static const string PORT_ANIMATION_INDEX		= "AnimationIndex";
	static const string PORT_AGENT_SCRIPT 			= "AgentScript";
	static const string PORT_INCLUDE_BARK 			= "IncludeBark";
	static const string PORT_BARK_MOOD 				= "BarkMood";
	static const string PORT_RELATED_INVOKER 		= "RelatedInvoker";

	ref SCR_AIAnimationScript m_AnimationScript;
	AICommunicationComponent m_AIComms;

	protected static ref TStringArray s_aVarsIn = {
		PORT_GESTURE_TYPE
	};

	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn() { return s_aVarsIn; }

	protected static ref TStringArray s_aVarsOut = {
		PORT_DURATION,
		PORT_ANIMATION_INDEX,
		PORT_AGENT_SCRIPT,
		PORT_INCLUDE_BARK,
		PORT_BARK_MOOD,
		PORT_RELATED_INVOKER
	};

	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut() { return s_aVarsOut; }

	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette() { return true; }

	override event void OnEnter(AIAgent owner)
	{
		m_AnimationScript = new SCR_AIAnimationScript();
		m_AIComms = owner.GetCommunicationComponent(); 
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_AIComms)
			return ENodeResult.FAIL;
		
		SCR_AIOrder_Gesture order = SCR_AIOrder_Gesture.Cast(m_AIComms.GetOrder(0));
		if (!order)
			return ENodeResult.FAIL;
		
		int orderValue;
		if (!GetVariableIn(PORT_GESTURE_TYPE, orderValue) || order.m_eGestureType != orderValue)
			return NodeError(this, owner, "Wrong gesture provided");
		
		SCR_ECharacterGestureType eGestureType;
		int iMood;
		SCR_AIIntegerOperations.DecodeTwo(orderValue, eGestureType, iMood);

		m_AnimationScript.SetAnimationActorName(owner.GetControlledEntity().GetName());
		vector idMatrix[4];
		Math3D.MatrixIdentity4(idMatrix);
		float duration = -1;
		bool includeBark;

		SCR_AIAnimationWaypointParameters parameters = new SCR_AIAnimationWaypointParameters();

		switch (eGestureType)
		{
			case SCR_ECharacterGestureType.SIGN 	:
			{
				SCR_AIAnimation_SignUp animation = new SCR_AIAnimation_SignUp();
				if (!animation.GetAnimDataFromConfig())
					break;

				duration = animation.m_CustomAnimData.m_fAnimationDuration;
				parameters.SetParameters(idMatrix, duration, animation);
				includeBark = true;
				break;
			}

			case SCR_ECharacterGestureType.CHEER	:
			{
				SCR_AIAnimation_Cheer animation = new SCR_AIAnimation_Cheer();
				if (!animation.GetAnimDataFromConfig())
					break;

				duration = animation.m_CustomAnimData.m_fAnimationDuration;
				parameters.SetParameters(idMatrix, duration, animation);
				includeBark = true;
				break;
			}

			case SCR_ECharacterGestureType.BOO		:
			{
				SCR_AIAnimation_Boo animation = new SCR_AIAnimation_Boo();
				if (!animation.GetAnimDataFromConfig())
					break;

				duration = animation.m_CustomAnimData.m_fAnimationDuration;
				parameters.SetParameters(idMatrix, duration, animation);
				includeBark = true;
				break;
			}

			case SCR_ECharacterGestureType.PUSH		:
			{
				SCR_AIAnimation_Recovery_Push animation = new SCR_AIAnimation_Recovery_Push();
				if (!animation.GetAnimDataFromConfig())
					break;

				duration = animation.m_CustomAnimData.m_fAnimationDuration;
				parameters.SetParameters(idMatrix, duration, animation);
				break;
			}

			case SCR_ECharacterGestureType.PULL		:
			{
				SCR_AIAnimation_Recovery_Pull animation = new SCR_AIAnimation_Recovery_Pull();
				if (!animation.GetAnimDataFromConfig())
					break;

				duration = animation.m_CustomAnimData.m_fAnimationDuration;
				parameters.SetParameters(idMatrix, duration, animation);
				break;
			}

			case SCR_ECharacterGestureType.CANCEL		:
			{
				return ENodeResult.FAIL; //this will consume the CANCEL ORDER
				break;
			}
		}

		m_AnimationScript.AddAnimationWaypointParameter(parameters, 0);

		if (order.GetSender() == owner)
		{
			//order has been sent by Idle behavior.
			SCR_ChimeraAIBystander bystander = SCR_ChimeraAIBystander.Cast(owner);
			if (!bystander || bystander.IsInvokerDisabled())
				return ENodeResult.FAIL;	// the related invoker has been Invoked between issuing the Gesture order and executing it
		}
		
		if (duration < 0)
			duration = float.MAX;

		SetVariableOut(PORT_DURATION, duration);
		SetVariableOut(PORT_ANIMATION_INDEX, 0);
		SetVariableOut(PORT_AGENT_SCRIPT, m_AnimationScript);
		SetVariableOut(PORT_INCLUDE_BARK, includeBark);
		SetVariableOut(PORT_BARK_MOOD, iMood);
		SetVariableOut(PORT_RELATED_INVOKER, order.m_RelatedInvoker);

		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	override static event protected string GetOnHoverDescription()
	{
		return "SCR_AIGetGestureParameters reads animations data from config and returns values for task animate.";
	}
}
