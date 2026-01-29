/*!
SCR_AICommsHandler synchronizes AI talking and sends requests to VON component.


*/

void SCR_AICommsStateChangedDelegate(SCR_EAICommunicationState oldState, SCR_EAICommunicationState newState);
typedef func SCR_AICommsStateChangedDelegate;
typedef ScriptInvokerBase<SCR_AICommsStateChangedDelegate> SCR_AIOnCommsStateChangedInvoker;

enum SCR_EAICommunicationState
{
	IDLE,			// Not transmitting
	WAITING,		// Waiting for channel to be free to transmit
	TRANSMITTING,	// Transmitting
	SUSPENDED		// Suspended, like sleeping
};

class SCR_AICommsHandler : Managed
{
	// Data of this speaker
	IEntity m_Entity;
	AIAgent m_Agent;
	SignalsManagerComponent m_SignalsManagerComponent;
	VoNComponent m_VoNComponent;
	FactionAffiliationComponent m_FactionComp;
	
	protected ref array<ref SCR_AITalkRequest> m_aRequestQueue = {};	// Queue of requests, new requests are added to queue end
	protected ref ref SCR_AITalkRequest m_CurrentRequest;				// Current request which we are transmitting
	protected float m_fActiveTimer_ms;									// How much time we've been in active state
	bool m_bNeedUpdate = false;											// READ ONLY - check if this is true, and if so, call update
	protected SCR_EAICommunicationState m_eState;
	protected bool m_bMuted;
	
	protected const float SAMPLE_LENGTH_MS = 2000.0;			// Length of the sound sample. TODO: read the value from the data
	protected const float NEARBY_SPEAKER_CHECK_RANGE = 10.0;	// Distance for nearby speakers check
	
	
	//-------------------------------------------------------------------------------------------------------------
	// PUBLIC
	
	//-------------------------------------------------------------------------------------------------------------
	void SCR_AICommsHandler(notnull IEntity entity, notnull AIAgent agent)
	{
		m_Entity = entity;
		m_Agent = agent;
		m_VoNComponent = VoNComponent.Cast(entity.FindComponent(VoNComponent));
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(entity.FindComponent(SignalsManagerComponent));
		m_FactionComp = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
	}
	
	//-------------------------------------------------------------------------------------------------------------
	void AddRequest(SCR_AITalkRequest request)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("AddRequest: %1", request.GetDebugString()));
		#endif
		
		// Ignore and fail if muted
		if (m_bMuted)
		{
			#ifdef AI_DEBUG
			AddDebugMessage("  Ignored because CommsHandler is muted");
			#endif
			
			FailRequest(request);
			return;
		}
			
		// Check if we can bypass the request, and if yes, instantly complete it
		/*
		if (m_eState != SCR_EAICommunicationState.SUSPENDED && CanBypass(request))
		{
			#ifdef AI_DEBUG
			AddDebugMessage("  Bypassed the request");
			#endif
			
			CompleteRequest(request);
			return;
		}
		*/
		
		// Add request, sort by priority
		int newPriority = request.m_iPriority;
		
		int idInsertAt = 0;
		for (int i = m_aRequestQueue.Count()-1; i >= 0; i--)
		{
			int priority = m_aRequestQueue[idInsertAt].m_iPriority;
			
			if (newPriority > priority)
			{
				continue;
			}
			else
			{
				// Same priority or lower than this - add behind this request
				idInsertAt = i+1;
				break;
			}
		}
		
		if (idInsertAt == m_aRequestQueue.Count())
			m_aRequestQueue.Insert(request);
		else
			m_aRequestQueue.InsertAt(request, idInsertAt);
		
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("  Added to queue at: %1, new queue size: %2", idInsertAt, m_aRequestQueue.Count()));
		#endif
		
		// We must be updated from now on
		m_bNeedUpdate = true;
	}
	
	//-------------------------------------------------------------------------------------------------------------
	//! Checks if we can optimize out transmitting this, which results in successful completion of request
	//! You should call this before creating a talk request and adding it to the queue
	//! request - optional, the request which we will check if we can bypass it. If it's not passed, oinly generic rules are checked.
	bool CanBypass(SCR_AITalkRequest request = null)
	{
		// Bypass if LOD is too high
		if (m_Agent.GetLOD() != 0 || !m_Agent.GetControlComponent().IsAIActivated())
			return true;
		
		// Bypass if there is no VoN component
		if (!m_VoNComponent)
			return true;
		
		// Bypass if noone's listening and the requests allows it
		AIGroup myGroup = m_Agent.GetParentGroup();
		if (request)
		{
			if ((!myGroup || myGroup.GetAgentsCount() <= 1) &&
				!request.m_bTransmitIfNoReceivers)
				return true;
			if (!request.m_bTransmitIfPassenger)
			{
				ChimeraCharacter char = ChimeraCharacter.Cast(m_Agent.GetControlledEntity());
				if (char && char.IsInVehicle())
				{
					CompartmentAccessComponent compAcc = char.GetCompartmentAccessComponent();
					if (compAcc && !TurretCompartmentSlot.Cast(compAcc.GetCompartment()))
						return true;
				}
			}
		}
		
		// Bypass if we are a leader of slave group
		SCR_AIGroup scrAiGroup = SCR_AIGroup.Cast(myGroup);
		if (scrAiGroup && scrAiGroup.IsSlave() && scrAiGroup.GetLeaderAgent() == m_Agent)
			return true;
		
		return false;
	}
	
	//-------------------------------------------------------------------------------------------------------------
	void SetMuted(bool mute)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetMuted: %1", mute));
		#endif
		
		if (mute)
		{
			if (m_eState == SCR_EAICommunicationState.TRANSMITTING || m_eState == SCR_EAICommunicationState.WAITING)
			{
				// Fail all requests
				ClearAndFailAllRequests();
				
				m_bNeedUpdate = false;
				
				SwitchToState(SCR_EAICommunicationState.IDLE);
			}
		}
		
		m_bMuted = mute;				
	}
	
	//-------------------------------------------------------------------------------------------------------------
	bool GetMuted()
	{
		return m_bMuted;
	}

	//-------------------------------------------------------------------------------------------------------------
	//! 'Suspended' means a sleeping state
	//! When switching to suspended state, current request, if exists, is put back into queue.
	//! Requests are not served in suspended state.
	void SetSuspended(bool suspended)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetSuspended: %1", suspended));
		#endif
		
		if (suspended)
		{
			SwitchToState(SCR_EAICommunicationState.SUSPENDED);
			
			// Put current request into front of the queue, so we return to it later
			if (m_CurrentRequest)
			{
				m_aRequestQueue.InsertAt(m_CurrentRequest, 0);
				m_CurrentRequest = null;
			}	
		}
		else
		{
			if (m_eState == SCR_EAICommunicationState.SUSPENDED)
			{
				SwitchToState(SCR_EAICommunicationState.IDLE);
			}
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------
	bool GetSuspended()
	{
		return m_eState == SCR_EAICommunicationState.SUSPENDED;
	}
	
	//-------------------------------------------------------------------------------------------------------------
	// UDPATE
	
	//-------------------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		switch (m_eState)
		{
			case SCR_EAICommunicationState.IDLE:
			{
				// Find top priority request which is still valid
				SCR_AITalkRequest validRequest = FindValidRequest();
				
				// Found a valid request
				if (validRequest)
				{
					m_CurrentRequest = validRequest;
					TryTransmitAndSwitchState();
				}
				
				break;
			}
			
			case SCR_EAICommunicationState.WAITING:
			{
				// Is it still valid?
				float currentTime_ms = GetGame().GetWorld().GetWorldTime();
				
				if (currentTime_ms - m_CurrentRequest.m_fCreatedTimestamp_ms >= m_CurrentRequest.m_fTimeout_ms)
				{
					// This request is not valid any more, forget it
					FailRequest(m_CurrentRequest);
					m_CurrentRequest = null;
					SwitchToState(SCR_EAICommunicationState.IDLE);
				}
				else
				{
					// Still valid, check if we can talk
					if (CanTransmit(m_CurrentRequest))
					{
						TransmitRequest(m_CurrentRequest);
						SwitchToState(SCR_EAICommunicationState.TRANSMITTING);
						m_fActiveTimer_ms = 0;
					}
				}
				
				break;
			}
			
			case SCR_EAICommunicationState.TRANSMITTING:
			{					
				m_fActiveTimer_ms += timeSlice;
				
				if (m_fActiveTimer_ms > SAMPLE_LENGTH_MS)
				{
					// Back to IDLE
					CompleteRequest(m_CurrentRequest);
					m_CurrentRequest = null;
					
					// Immediately try to find next request
					SCR_AITalkRequest validRequest = FindValidRequest();
					
					if (validRequest)
					{
						m_CurrentRequest = validRequest;
						if (m_CurrentRequest.m_bTransmitIfChannelBusy)
						{
							// If this request can be transmitted over busy channel, immediately start transmitting it,
							// we want to keep channel busy, without making it free, so others don't transmit
							TryTransmitAndSwitchState();
						}
						else
						{
							// If we can't transmit this over busy channel, free channel for one update, let others transmit
							SwitchToState(SCR_EAICommunicationState.WAITING);
						}
					}
					else
					{
						SwitchToState(SCR_EAICommunicationState.IDLE);
					}
				}
				
				break;
			}
		}
		
		
		// Update me more if queue is not empty, or there is a current request
		m_bNeedUpdate = !m_aRequestQueue.IsEmpty() || m_CurrentRequest;
	}
	
	//-------------------------------------------------------------------------------------------------------------
	//! Fails and clears all requests, resets to initial state
	void Reset()
	{
		#ifdef AI_DEBUG
		AddDebugMessage("Reset");
		#endif
		ClearAndFailAllRequests();
		SwitchToState(SCR_EAICommunicationState.IDLE);
	}
	
	//-------------------------------------------------------------------------------------------------------------
	// PROTECTED / PRIVATE
	
	
	//-------------------------------------------------------------------------------------------------------------
	// Finds highest priority valid request, removes invalid requests along the way
	protected SCR_AITalkRequest FindValidRequest()
	{
		float currentTime_ms = GetGame().GetWorld().GetWorldTime();
		SCR_AITalkRequest request;
		for (int i = 0; i < m_aRequestQueue.Count(); i++)
		{
			SCR_AITalkRequest thisRequest = m_aRequestQueue[0];
			if (currentTime_ms - thisRequest.m_fCreatedTimestamp_ms < thisRequest.m_fTimeout_ms)
			{
				request = thisRequest;
				m_aRequestQueue.RemoveOrdered(0);
				break;
			}
			else
			{
				FailRequest(thisRequest);
				m_aRequestQueue.RemoveOrdered(0);
			}
		}
		
		return request;
	}
	
	//-------------------------------------------------------------------------------------------------------------
	//! Tries to transmit, if succeedes, switches to ACTIVE state, otherwise to WAITING state
	protected void TryTransmitAndSwitchState()
	{
		if (CanBypass(m_CurrentRequest))
		{
			// Check if we can bypass this
			CompleteRequest(m_CurrentRequest);
			m_CurrentRequest = null;
			SwitchToState(SCR_EAICommunicationState.IDLE);
		}
		else if (CanTransmit(m_CurrentRequest))
		{
			// Check if we can talk right now
			TransmitRequest(m_CurrentRequest);
			SwitchToState(SCR_EAICommunicationState.TRANSMITTING);
			m_fActiveTimer_ms = 0;
		}
		else
		{
			// We'll be back soon!
			SwitchToState(SCR_EAICommunicationState.WAITING);
		}
	}
	
	
	
	//-------------------------------------------------------------------------------------------------------------
	// Transmission rules
	
	//-------------------------------------------------------------------------------------------------------------
	//! Checks if we can transmit with relation to channel state
	protected bool CanTransmit(SCR_AITalkRequest request)
	{
		if (request.m_bTransmitIfChannelBusy)
		{
			// We can always transmit this request, don't care if channel is busy
			return true;
		}
		else
		{
			return IsChannelFree();
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------
	//! Checks if channel is free for transmission.
	//! Now it checks nearby characters.
	protected bool IsChannelFree()
	{	
		ChimeraWorld world = m_Entity.GetWorld();
		if (!world)
			return true;
		
		array<IEntity> entities = {};
		TagSystem tm = TagSystem.Cast(world.FindSystem(TagSystem));
		if (!tm)
		{
			Print("SCR_AICommsHandler: TagManager is not present in the world, AI comms might behave incorrect", LogLevel.ERROR);
			return true;
		}
			
		tm.GetTagsInRange(entities, m_Entity.GetOrigin(), NEARBY_SPEAKER_CHECK_RANGE, ETagCategory.NameTag);
		
		Faction myFaction;
		if (m_FactionComp)
			myFaction = m_FactionComp.GetAffiliatedFaction();
		foreach (IEntity ent : entities)
		{
			SCR_ChimeraCharacter characterEnt = SCR_ChimeraCharacter.Cast(ent);
			if (!characterEnt)
				continue;
			
			// Ignore if different faction
			if (characterEnt.m_pFactionComponent && (characterEnt.m_pFactionComponent.GetAffiliatedFaction() != myFaction))
				continue;
			
			CharacterControllerComponent characterController = characterEnt.GetCharacterController();			
			AIControlComponent aiControlComponent = characterController.GetAIControlComponent();
			
			if (!aiControlComponent)
				continue;
			
			AIAgent agent = aiControlComponent.GetControlAIAgent();			
			SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(agent);
			if (!chimeraAgent)
				continue;
			
			SCR_AIUtilityComponent utility = chimeraAgent.m_UtilityComponent;
			
			if (utility.m_CommsHandler.m_eState == SCR_EAICommunicationState.TRANSMITTING)
				return false;	
		}
		
		return true;
	}
	
	// Transmission rules
	//-------------------------------------------------------------------------------------------------------------
	
	
	
	//-------------------------------------------------------------------------------------------------------------
	protected void SwitchToState(SCR_EAICommunicationState newState)
	{
		if (newState == m_eState)
			return;
		
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SwitchToState: %1", typename.EnumToString(SCR_EAICommunicationState, newState)));
		#endif
		
		SCR_EAICommunicationState oldState = m_eState;
		m_eState = newState;
	}
	
	//-------------------------------------------------------------------------------------------------------------
	protected void TransmitRequest(SCR_AITalkRequest rq)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("TransmitRequest: %1", rq.GetDebugString()));
		#endif
		
		rq.m_eState = SCR_EAITalkRequestState.TRANSMITTING;
		bool txSuccess = SCR_AISoundHandling.SetSignalsAndTransmit(rq, m_Entity, m_VoNComponent, m_SignalsManagerComponent);
		
		if (!txSuccess)
		{
			string str = string.Format("SCR_AISoundHandling.SetSignalsAndTransmit failed for request: %1", rq.GetDebugString());

			Print(string.Format("SCR_AICommsHandler: %1", str), LogLevel.ERROR);
						
			#ifdef AI_DEBUG
			AddDebugMessage(str, LogLevel.ERROR);
			#endif
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------
	protected void FailRequest(SCR_AITalkRequest rq)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("FailRequest: %1", rq.GetDebugString()));
		#endif
		rq.m_eState = SCR_EAITalkRequestState.FAILED;
	}
	
	//-------------------------------------------------------------------------------------------------------------
	protected void CompleteRequest(SCR_AITalkRequest rq)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("CompleteRequest: %1", rq.GetDebugString()));
		#endif
		rq.m_eState = SCR_EAITalkRequestState.COMPLETED;
	}
	
	//-------------------------------------------------------------------------------------------------------------
	//! Clears and fails all requests
	protected void ClearAndFailAllRequests()
	{
		#ifdef AI_DEBUG
		AddDebugMessage("ClearAndFailAllRequests");
		#endif
		
		if (m_CurrentRequest)
			FailRequest(m_CurrentRequest);
		m_CurrentRequest = null;
		
		foreach (SCR_AITalkRequest request : m_aRequestQueue)
			FailRequest(request);
		m_aRequestQueue.Clear();
		
		SwitchToState(SCR_EAICommunicationState.IDLE);
	}
	
	//--------------------------------------------------------------------------------------------
	void EOnDiag(float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_COMMS_HANDLERS))
		{	
			vector ownerPos = m_Entity.GetOrigin();
			vector textPos = ownerPos + Vector(0, 4.5, 0);
			int color = Color.GRAY;
			
			string str = string.Format("Queue: %1, State: %2",
				m_aRequestQueue.Count(),
				typename.EnumToString(SCR_EAICommunicationState, m_eState));
			
			if(m_CurrentRequest)
			{
				if (m_CurrentRequest.m_eState == SCR_EAITalkRequestState.TRANSMITTING)
					color = Color.MAGENTA;
				else
					color = Color.ORANGE;
				
				str += string.Format("\n%1", typename.EnumToString(ECommunicationType, m_CurrentRequest.m_eCommType));
			}
				
			DebugTextWorldSpace.Create(GetGame().GetWorld(), str, DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
					textPos[0], textPos[1], textPos[2], color: color, bgColor: Color.BLACK,
					size: 13.0); 
		}
	}
	
	//--------------------------------------------------------------------------------------------
	void ~SCR_AICommsHandler()
	{
		ClearAndFailAllRequests();
	}
	
	#ifdef AI_DEBUG
	//--------------------------------------------------------------------------------------------
	protected void AddDebugMessage(string str, LogLevel logLevel = LogLevel.NORMAL)
	{
		if (!m_Agent)
			return;
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(m_Agent.FindComponent(SCR_AIInfoBaseComponent));
		if (!infoComp)
			return;
		
		infoComp.AddDebugMessage(str, msgType: EAIDebugMsgType.COMMS);
	}
	#endif
}