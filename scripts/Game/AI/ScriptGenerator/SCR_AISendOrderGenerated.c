class SCR_AISendOrderGenerated : AITaskScripted
{
	static const string PORT_RECEIVER = "Receiver";
	
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sText;
		
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	protected bool SendMessage(AIAgent owner, AIAgent receiver, AIMessage msg)
	{
		AICommunicationComponent comms = owner.GetCommunicationComponent();
		if (!comms || !receiver)
			return false;
		
		return comms.RequestBroadcast(msg, receiver);
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