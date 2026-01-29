enum SCR_EAITalkRequestState
{
	IDLE,			// Waiting in queue or waiting for free channel
	TRANSMITTING,	// Currently transmitting
	COMPLETED,		// Was transmitted
	FAILED			// Failed to transmit
}

enum SCR_EAITalkRequestPreset
{
	IRRELEVANT_IMMEDIATE,	// Small priority, tiny timeout. For almost irrelevant phrases, which become irrelevant in a second.
	IRRELEVANT,				// Small priority, small timeout. For almost irrelevant phrases.
	MEDIUM,					// Medium priority, medium timeout.
	MANDATORY,				// Highest priority, infinite timeout. For phrases related to orders and similar.
	IMMEDIATE				// Same as MANDATORY, but also ignores channel state
}

class SCR_AITalkRequest : Managed
{	
	// Properties from SCR_AISoundHandlingParams
	ECommunicationType m_eCommType;
	IEntity m_Entity;
	vector m_vPosition;	
	int m_EnumSignal;
	
	float m_fCreatedTimestamp_ms;				// Time when it was added
	float m_fTimeout_ms;						// Request is discarded if we were not able to start saying this during this interval
	int m_iPriority;							// Priority of request, higher priority gets processed sooner
	bool m_bTransmitIfChannelBusy;				// If true, we will not care to wait for channel to be free and transmit over others
	bool m_bTransmitIfNoReceivers;				// If true, will transmit even if we know noone's listening
	bool m_bTransmitIfPassenger;				// If true, will transmit if inside vehicle as passenger (gunner is not affected)
	
	SCR_EAITalkRequestState m_eState;			// READ ONLY - CommsHandler sets this when it processes the request. This can be read to know the outcome of request.
	
	//-------------------------------------------------------------------------------------
	// Some parameters are initialized in constructor, others from InitFromPreset
	void SCR_AITalkRequest(ECommunicationType type, IEntity entity, vector pos, int enumSignal, bool transmitIfNoReceivers, bool transmitIfPassenger, SCR_EAITalkRequestPreset preset)
	{
		m_fCreatedTimestamp_ms = GetGame().GetWorld().GetWorldTime();
		
		m_eCommType = type;
		m_Entity = entity;
		m_vPosition = pos;
		m_EnumSignal = enumSignal;
		m_bTransmitIfNoReceivers = transmitIfNoReceivers;
		m_bTransmitIfPassenger = transmitIfPassenger;
		
		InitFromPreset(preset);
	}
	
	//-------------------------------------------------------------------------------------
	//! Initializes timeout, priority, transmitIfBusy according to preset
	void InitFromPreset(SCR_EAITalkRequestPreset preset)
	{
		int prio;
		float timeout;
		bool transmitIfBusy = false;
		switch (preset)
		{
			case SCR_EAITalkRequestPreset.IRRELEVANT_IMMEDIATE:
			{ 
				prio = 5;
				timeout = 500.0;
				break;
			}
			
			case SCR_EAITalkRequestPreset.IRRELEVANT:
			{ 
				prio = 10;
				timeout = 4000.0;
				break;
			}
			
			case SCR_EAITalkRequestPreset.MEDIUM:
			{
				prio = 20;
				timeout = 10000.0;
				break;
			}
			
			case SCR_EAITalkRequestPreset.MANDATORY:
			{
				prio = 30;
				timeout = float.MAX;
				break;
			}
			
			case SCR_EAITalkRequestPreset.IMMEDIATE:
			{
				prio = 40;
				timeout = float.MAX;
				transmitIfBusy = true;
				break;
			}
		}
		
		m_iPriority = prio;
		m_fTimeout_ms = timeout;
		m_bTransmitIfChannelBusy = transmitIfBusy;
	}
	
	string GetDebugString()
	{
		return string.Format("%1, prio: %2, timeout: %3, txIfBusy: %4, type: %5",
			this,
			m_iPriority,
			m_fTimeout_ms,
			m_bTransmitIfChannelBusy,
			typename.EnumToString(ECommunicationType, m_eCommType));
	}
}