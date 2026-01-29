class SCR_AIMessageSelectionMenuEntry : AIMessageSelectionMenuEntry
{
	private AICommunicationComponent m_ParentComms;
	private AIAgent m_Receiver;
	
	void SetParentComms(AICommunicationComponent parentComms, AIAgent receiver)
	{
		m_ParentComms = parentComms;
		m_Receiver = receiver;
	}
	
	//! Callback for when this entry is supposed to be performed
	protected override event void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		super.OnPerform(user, sourceMenu);
		if (m_ParentComms)
		{
			m_ParentComms.RequestBroadcast(GetAIMessage(), m_Receiver);
		}
	}

	protected override bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		// TODO: custom show logic
		return true;
	}

	protected override bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		
		// TODO: custom perform logic
		return true;
	}
	
	protected override bool GetEntryNameScript(out string outName)
	{
		auto pMessage = GetAIMessage();
		if (!pMessage)
		{
			outName = "Command";
			return true;
		}
		
		string sName = pMessage.GetText();
		outName = sName;
		return true;
	}
	
	protected override bool GetEntryDescriptionScript(out string outDescription)
	{
		return false;
	}
};