class SCR_TaskRadioMsg : ScriptedRadioMessage
{		
	protected SCR_ETaskNotification m_eTaskNotificationType;
	protected SCR_ETaskNotificationMsg m_eTaskNotificationMsg;
	protected SCR_BaseTaskNotificationData m_Data;
	protected int m_iBaseCallsign =  SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;	
	protected int m_iCallerGroupId;
	protected int m_iCalledGroupId;
	protected int m_iGrid;
	protected int m_iCallerPlayerId;
	protected int m_iFactionId;
	protected SCR_Faction m_Faction;
	protected vector m_vTaskPosition;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] type
	//! \param[in] msg
	void SetTaskNotification(SCR_ETaskNotification type, SCR_ETaskNotificationMsg msg)
	{
		m_eTaskNotificationType = type;
		m_eTaskNotificationMsg = msg;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] data
	void SetTaskNotificationData(SCR_BaseTaskNotificationData data)
	{
		m_Data = data;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] callsign
	void SetBaseCallsign(int callsign)
	{
		m_iBaseCallsign = callsign;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] groupId
	void SetCallerGroupId(int groupId)
	{
		m_iCallerGroupId = groupId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] groupId
	void SetCalledGroupId(int groupId)
	{
		m_iCalledGroupId = groupId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] grid
	void SetGrid(int grid)
	{
		m_iGrid = grid;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] callerPlayerId
	void SetCallerPlayerId(int callerPlayerId)
	{
		m_iCallerPlayerId = callerPlayerId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] factionId
	void SetFactionId(int factionId)
	{
		m_iFactionId = factionId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	void SetFaction(SCR_Faction faction)
	{
		m_Faction = faction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] taskPosition
	void SetTaskPosition(vector taskPosition)
	{
		m_vTaskPosition = taskPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelivery(BaseTransceiver receiver, int freq, float quality)
	{
		IEntity owner = receiver.GetRadio().GetOwner();
		ChimeraCharacter player;

		while (!player)
		{
			player = ChimeraCharacter.Cast(owner);			
			if (player)
				break;
			
			owner = owner.GetParent();			
			if (!owner)
				return;
		}

		if (player)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(owner);
			
			// skips caller player, He should not receive messages sent by himself. I think this check was not needed in the past, maybe there is a bug somewhere.
			if (playerID == m_iCallerPlayerId)
				return;
						
			if (!m_Data || !m_Data.CanSendToPlayer(m_Faction, playerID, m_vTaskPosition))
				return;		
			
			PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);			
			if (!playerController)
				return;		
			
			SCR_CampaignNetworkComponent networkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
			if (networkComponent)
				networkComponent.PlayTaskNotificationMsg(m_eTaskNotificationType, m_eTaskNotificationMsg, m_iFactionId, m_iBaseCallsign, m_iCallerGroupId, m_iCalledGroupId, m_iGrid, quality, 0);
		}		
	}
};