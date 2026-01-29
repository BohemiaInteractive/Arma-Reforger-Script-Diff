//! Base class for nodes sending message.
class SCR_AISendMessageBase: AITaskScripted
{
	// Constants
	static const string PORT_RECEIVER = "Receiver";
	static const string PORT_STRING = "StringIn";
	
	// Variables
	protected AICommunicationComponent m_Mailbox;
	AIAgent m_Receiver;
	
	[Attribute("", UIWidgets.EditBox, "String value", "")]
	string m_string;
	
	
	protected static ref TStringArray s_aVarsInBase = {
		PORT_RECEIVER,
		PORT_STRING
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsInBase;
    }
	
	override void OnInit(AIAgent owner)
	{
		m_Mailbox = owner.GetCommunicationComponent();
	}
	
	// Validates owner, mailbox, reads RECEIVER port.
	protected bool InitSendMessageInputs(AIAgent owner)
	{
		if (! owner)
		{
			Print("No owner of the task send message! Something wrong!", LogLevel.ERROR);
			return false;
		};
		
		if (!m_Mailbox)
		{
			Print("Owner of the task has no AICommunicationComponent!", LogLevel.ERROR);
			return false;
		};
		
		GetVariableIn(PORT_RECEIVER,m_Receiver);
		if (! m_Receiver)
		{
			m_Receiver = owner;
		};
		return true;
	}
	
	//! Calls this in inherited nodes to send the message
	protected ENodeResult SendMessage(AIAgent owner, AIMessage msg)
	{
		#ifdef AI_DEBUG
		SCR_MailboxComponent mailboxComp = SCR_MailboxComponent.Cast(m_Mailbox);
		if (mailboxComp)
			mailboxComp.DebugLogBroadcastMessage(msg);
		#endif
		
		if (m_Mailbox.RequestBroadcast(msg, m_Receiver))
			return ENodeResult.SUCCESS;
		else
		{
			PrintFormat("Unable to send message from %1 to %2", owner, m_Receiver);
			return ENodeResult.FAIL;
    	};
	}
};

//! Class for generic nodes with extra input ports which send messages
class SCR_AISendMessageGeneric: SCR_AISendMessageBase
{	
	static const string PORT_ENTITY = "EntityIn";
	static const string PORT_INTEGER = "IntegerIn";
	static const string PORT_VECTOR = "VectorIn";
	static const string PORT_BOOL = "BoolIn";
	static const string PORT_FLOAT = "FloatIn";	
	static const string PORT_TYPENAME = "TypenameIn";
	static const string PORT_SMARTACTION = "SmartActionComponent";
	static const string PORT_PRIORITY_LEVEL = "PriorityLevel";
	
	
	[Attribute("0", UIWidgets.EditBox, "Integer value", "")]
	int m_integer;
	
	[Attribute("0", UIWidgets.EditBox, "Vector value", "")]
	vector m_vector;
		
	[Attribute("0", UIWidgets.CheckBox, "Bool value", "")]
	bool m_bool;
	
	protected SCR_AIWorld m_aiWorld;
	
	// Sets up input variables, as array of strings
	protected static ref TStringArray s_aVarsIn = {
		PORT_RECEIVER,
		PORT_STRING,
		PORT_ENTITY,
		PORT_INTEGER,
		PORT_VECTOR,
		PORT_BOOL,
		PORT_FLOAT,
		PORT_TYPENAME,
		PORT_PRIORITY_LEVEL
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (InitSendMessageInputs(owner))
			return ENodeResult.SUCCESS;
		
		return ENodeResult.FAIL;
	}
	
	// Also validates AIWorld
	override protected bool InitSendMessageInputs(AIAgent owner)
	{
		if (!super.InitSendMessageInputs(owner))
			return false;
		
		if(!m_aiWorld)
		{
			m_aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
			if (!m_aiWorld)
			{
				Print("Cannot find AIWorld to read config of messages!", LogLevel.ERROR);
				return false;
			}
		};
		
		return true;
	}
	
	static override bool VisibleInPalette()
	{
		return false;
	}
};
