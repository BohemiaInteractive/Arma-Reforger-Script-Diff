class SCR_AITalk: AITaskScripted
{
	static const string PORT_SPEAKER = "SpeakerIn";
	static const string PORT_TARGET = "TargetIn";
	static const string PORT_LOCATION = "LocationIn";
	static const string PORT_INT = "EnumIn";
	
	[Attribute("0", UIWidgets.ComboBox, "Message type", "", ParamEnumArray.FromEnum(ECommunicationType) )]
	protected ECommunicationType m_messageType;
	
	[Attribute("0", UIWidgets.ComboBox, "Determines priority, timeout, and other values. See SCR_EAITalkRequestPreset", enums: ParamEnumArray.FromEnum(SCR_EAITalkRequestPreset))]
	protected SCR_EAITalkRequestPreset m_ePreset;
	
	[Attribute("0", UIWidgets.CheckBox, "When true, the node will be running until the request is finished, and will fail if the request fails.")]
	protected bool m_bSynchronous;
	
	[Attribute("0", UIWidgets.CheckBox, "When true, request will be transmitted by speaker even if there is noone to potentially receive (hear) it.")]
	protected bool m_bTransmitIfNoReceivers;
	
	[Attribute("1", UIWidgets.CheckBox, "When true, request will be transmitted by speaker being a passanger of a vehicle.")]
	protected bool m_bTransmitIfPassenger;
	
	protected ref SCR_AITalkRequest m_Request;

	//-----------------------------------	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (m_Request)
		{
			// We've sent a request, check its state ...
			SCR_EAITalkRequestState rqState = m_Request.m_eState;
			if (rqState == SCR_EAITalkRequestState.IDLE || rqState == SCR_EAITalkRequestState.TRANSMITTING)
				return ENodeResult.RUNNING;
			else if (rqState == SCR_EAITalkRequestState.COMPLETED)
			{
				//_print(string.Format("Request completed: %1", m_Request.GetDebugString()));
				Reset();
				return ENodeResult.SUCCESS;
			}
			else if (rqState == SCR_EAITalkRequestState.FAILED)
			{
				//_print(string.Format("Request failed: %1", m_Request.GetDebugString()));
				Reset();
				return ENodeResult.FAIL;
			}
		}
		else
		{
			// Didn't send a request yet
			IEntity speaker;
			bool speakerPortConnected = GetVariableIn(PORT_SPEAKER, speaker);
			if (!speakerPortConnected)
			{
				// Port not connected - speaker is owner of node
				speaker = owner;
			}
			else if (speakerPortConnected && !speaker)
			{
				// Port is connected but entity is null, failure
				Reset();
				return ENodeResult.FAIL;
			}
			
			SCR_AICommsHandler commsHandler = FindCommsHandler(speaker);
			
			if (!commsHandler)
			{
				// Could not find comms handler, it's impossible to transmit
				string callstackStr;
				GetCallstackStr(callstackStr);
				Print(string.Format("SCR_AITalk: was not able to find SCR_AICommsHandler for %1. BT: %2", speaker, callstackStr), LogLevel.ERROR);
				Reset();
				return ENodeResult.FAIL;
			}
			
			// Check if we can bypass this (optimize out)
			if (commsHandler.CanBypass())
			{
				return ENodeResult.SUCCESS;
			}
				
			// Read ports
			IEntity rqTarget;
			vector rqPos;
			int rqEnum;
			
			GetVariableIn(PORT_TARGET, rqTarget);
			GetVariableIn(PORT_LOCATION, rqPos);
			GetVariableIn(PORT_INT, rqEnum);
			
			// Create and fill request
			SCR_AITalkRequest rq = new SCR_AITalkRequest(m_messageType, rqTarget, rqPos, rqEnum, m_bTransmitIfNoReceivers, m_bTransmitIfPassenger, m_ePreset);
			
			commsHandler.AddRequest(rq);
			
			//_print(string.Format("Added request: %1", rq.GetDebugString()));
			
			if (m_bSynchronous)
			{
				// If synchronous, we care about request result
				
				// Store request, return running, wait until it's done
				//_print("Synchronous, will wait for request result");
				m_Request = rq;
				return ENodeResult.RUNNING;
			}
			else
			{
				// If asynchronous, we don't care about request result
				//_print("Asynchronous, return success");
				Reset();
				return ENodeResult.SUCCESS;
			}
		}
		return 0;
	}
	
	//----------------------------------------------------------------------
	//! Finds comms handler of provided speaker
	protected SCR_AICommsHandler FindCommsHandler(notnull IEntity speaker)
	{
		AIAgent speakerAgent = AIAgent.Cast(speaker);
		AIGroup speakerGroup = AIGroup.Cast(speaker);
		if (speakerGroup)
		{
			// Speaker is group
			// Find comms handler of leader agent
			
			AIAgent leaderAgent = speakerGroup.GetLeaderAgent();
			
			if (!leaderAgent)
				return null;
			
			SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(leaderAgent.FindComponent(SCR_AIUtilityComponent));
			if (!utility)
				return null;
			
			return utility.m_CommsHandler;
		}	
		else if (speakerAgent)
		{
			SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(speakerAgent.FindComponent(SCR_AIUtilityComponent));
			if (!utility)
				return null;
			return utility.m_CommsHandler;
		}
		else
		{
			// Speaker is Entity
			AIControlComponent contr = AIControlComponent.Cast(speaker.FindComponent(AIControlComponent));
			if (!contr)
				return null;
			
			speakerAgent = contr.GetControlAIAgent();
			if (!speakerAgent)
				return null;
			
			SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(speakerAgent.FindComponent(SCR_AIUtilityComponent));
			if (!utility)
				return null;
			return utility.m_CommsHandler;
		}
	}

	//----------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		Reset();
	}
	
	//----------------------------------------------------------------------
	//! Resets internal state, should be called on abort and such
	protected void Reset()
	{
		m_Request = null;
	}
	
	//----------------------------------------------------------------------
	static override bool VisibleInPalette()
    {
        return true;
    }
	
	//----------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_SPEAKER,
		PORT_TARGET,
		PORT_LOCATION,
		PORT_INT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
	}
	
	
	//----------------------------------------------------------------------
	static override protected bool CanReturnRunning()
	{
		return true;
	}
	
	//----------------------------------------------------------------------
	protected void _print(string str)
	{
		Print(string.Format("SCR_AITalk %1: %2", this, str));
	}
};
