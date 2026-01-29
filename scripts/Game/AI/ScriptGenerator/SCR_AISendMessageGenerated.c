class SCR_AISendMessageGenerated : AITaskScripted
{
	static const string PORT_RECEIVER = "Receiver";
	
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sText;
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	protected SCR_AIActivityBase GetRelatedActivity(AIAgent owner)
	{
		SCR_AIBaseUtilityComponent utilityComp = SCR_AIBaseUtilityComponent.Cast(owner.FindComponent(SCR_AIBaseUtilityComponent));
		if (!utilityComp)
			return null;
		
		if (SCR_AIGroup.Cast(owner))
			return SCR_AIActivityBase.Cast(utilityComp.GetExecutedAction());
		else
		{
			SCR_AIBehaviorBase behavior = SCR_AIBehaviorBase.Cast(utilityComp.GetExecutedAction());
			if (behavior)
				return SCR_AIActivityBase.Cast(behavior.GetRelatedGroupActivity());
			else
				return null;
		}
		
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	protected AIWaypoint GetRelatedWaypoint(AIAgent owner)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
			return null;
		return group.GetCurrentWaypoint();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	protected bool SendMessage(AIAgent owner, AIAgent receiver, SCR_AIMessageBase msg)
	{
		AICommunicationComponent comms = owner.GetCommunicationComponent();
		if (!comms || !receiver)
			return false;
		
		#ifdef AI_DEBUG
		InitDebugData(msg.m_sSentFromBt);
		#endif
		
		return comms.RequestBroadcast(msg, receiver);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	protected bool SendMessage(AIAgent owner, AIAgent receiver, SCR_AIOrderBase msg)
	{
		AICommunicationComponent comms = owner.GetCommunicationComponent();
		if (!comms || !receiver)
			return false;
		
		#ifdef AI_DEBUG
		InitDebugData(msg.m_sSentFromBt);
		#endif
		
		return comms.RequestBroadcast(msg, receiver);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	protected void InitDebugData(out string outSentFromBt)
	{
		string parentTreeName;
		GetParentTreeName(parentTreeName);
		outSentFromBt = string.Format("%1, node: %2 %3", parentTreeName, GetNodeID(), this.Type());
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	protected AIAgent GetReceiverAgent(AIAgent owner)
	{
		AIAgent receiver;
		if (!GetVariableIn(PORT_RECEIVER, receiver))
			receiver = owner;
		
		return receiver;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return false; }	
}