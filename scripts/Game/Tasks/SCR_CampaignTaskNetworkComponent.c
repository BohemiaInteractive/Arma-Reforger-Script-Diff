//[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "Handles client > server communication in Campaign tasks. Should be attached to PlayerController.", color: "0 0 255 255")]
//class SCR_CampaignTaskNetworkComponentClass : SCR_TaskNetworkComponentClass
//{
//}
//
//class SCR_CampaignTaskNetworkComponent : SCR_TaskNetworkComponent
//{
//	
//	//------------------------------------------------------------------------------------------------
//	//!
//	void RequestReinforcements()
//	{
//		int playerID = GetPlayerId();
//		if (playerID == -1)
//			return;
//		
//		Rpc(RPC_RequestReinforcements, playerID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//!
//	//! \param[in] positionFrom
//	//! \param[in] positionTo
//	void RequestTransport(vector positionFrom, vector positionTo)
//	{
//		int playerID = GetPlayerId();
//		if (playerID == -1)
//			return;
//		
//		Rpc(RPC_RequestTransport, playerID, positionFrom, positionTo);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//!
//	//! \param[in] position
//	void RequestRefuel(vector position)
//	{
//		int playerID = GetPlayerId();
//		if (playerID == -1)
//			return;
//		
//		Rpc(RPC_RequestRefuel, playerID, position);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Allows the local player to request evacuation.
//	void RequestEvacuation(vector position)
//	{
//		int playerID = GetPlayerId();
//		if (playerID == -1)
//			return;
//		
//		Rpc(RPC_RequestEvacuation, playerID, position);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Requests refuel for the vehicle local player is currently in
//	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
//	protected void RPC_RequestReinforcements(int requesterID)
//	{
//		SCR_CampaignMilitaryBaseComponent base;
//		if (!SCR_CampaignDefendTask.CheckDefendRequestConditions(m_PlayerController, base))
//			return;
//		
//		if (!base)
//			return;
//		
//		SCR_CampaignNetworkComponent networkComponent = SCR_CampaignNetworkComponent.Cast(m_PlayerController.FindComponent(SCR_CampaignNetworkComponent));
//		if (!networkComponent)
//			return;
//		
//		networkComponent.SendPlayerMessage(SCR_ERadioMsg.REQUEST_REINFORCEMENTS, base.GetCallsign(), checkHQReached: true);
//		
//		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(m_PlayerController.GetMainEntity());
//		if (!character)
//		   return;
//		
//		SCR_CampaignFaction requesterFaction = SCR_CampaignFaction.Cast(character.GetFaction());
//		if (!requesterFaction)
//		   return;
//		
//		BaseRadioComponent radioComp = GetRadioComponent(m_PlayerController, character);
//		if (!radioComp)
//			return;
//
//		BaseTransceiver transmitter = radioComp.GetTransceiver(0);
//		if (!transmitter)
//			return;
//		
//		// send message with requester faction and his ID
//		SCR_RequestReinforcementsMessage msg = new SCR_RequestReinforcementsMessage();
//		msg.SetTargetFaction(requesterFaction);
//		msg.SetTargetBase(base);
//		//msg.SetRequesterID(requesterID);
//		transmitter.BeginTransmission(msg);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
//	protected void RPC_RequestTransport(int requesterID, vector positionFrom, vector positionTo)
//	{
//		if (GetTaskManager() && GetTaskManager().HasRequestedTask(requesterID))
//			return;
//		
//		SCR_CampaignNetworkComponent networkComponent = SCR_CampaignNetworkComponent.Cast(m_PlayerController.FindComponent(SCR_CampaignNetworkComponent));
//		if (!networkComponent)
//			return;
//		
//		networkComponent.SendPlayerMessage(SCR_ERadioMsg.REQUEST_TRANSPORT, checkHQReached: true);
//		
//		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(m_PlayerController.GetMainEntity());
//		if (!character)
//		   return;
//		
//		SCR_CampaignFaction requesterFaction = SCR_CampaignFaction.Cast(character.GetFaction());
//		if (!requesterFaction)
//		   return;
//
//		BaseRadioComponent radioComp = GetRadioComponent(m_PlayerController, character);
//		if (!radioComp)
//			return;
//
//		BaseTransceiver transmitter = radioComp.GetTransceiver(0);
//		if (!transmitter)
//			return;
//
//		// send message with requester faction and his ID
//		SCR_RequestTransportMessage msg = new SCR_RequestTransportMessage();
//		msg.SetRequesterMainBase(requesterFaction);
//		msg.SetRequesterID(requesterID);
//		msg.SetPosition(positionFrom);
//		msg.SetTargetPosition(positionTo);
//		transmitter.BeginTransmission(msg);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Requests refuel for the vehicle local player is currently in
//	//! \param[in] requesterID
//	//! \param[in] position
//	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
//	protected void RPC_RequestRefuel(int requesterID, vector position)
//	{
//		if (GetTaskManager() && GetTaskManager().HasRequestedTask(requesterID))
//			return;
//		
//		SCR_CampaignNetworkComponent networkComponent = SCR_CampaignNetworkComponent.Cast(m_PlayerController.FindComponent(SCR_CampaignNetworkComponent));
//		if (!networkComponent)
//			return;
//		
//		networkComponent.SendPlayerMessage(SCR_ERadioMsg.REQUEST_FUEL, checkHQReached: true);
//		
//		SCR_BaseTaskExecutor taskExecutor = SCR_BaseTaskExecutor.GetTaskExecutorByID(requesterID);
//		if (!taskExecutor)
//			return;
//		
//		Vehicle vehicle = SCR_RefuelTask.GetVehicleExecutorIsIn(taskExecutor);
//		if (!vehicle)
//			return;
//		
//		if (!SCR_RefuelTask.CheckRefuelRequestConditions(taskExecutor))
//			return;
//		
//		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(m_PlayerController.GetMainEntity());
//		if (!character)
//		   return;
//		
//		SCR_CampaignFaction requesterFaction = SCR_CampaignFaction.Cast(character.GetFaction());
//		if (!requesterFaction)
//		   return;
//
//		BaseRadioComponent radioComp = GetRadioComponent(m_PlayerController, character);
//		if (!radioComp)
//			return;
//
//		BaseTransceiver transmitter = radioComp.GetTransceiver(0);
//		if (!transmitter)
//			return;
//
//		// send message with requester faction and his ID
//		SCR_RequestRefuelMessage msg = new SCR_RequestRefuelMessage();
//		msg.SetRequesterMainBase(requesterFaction);
//		msg.SetRequesterID(requesterID);
//		msg.SetPosition(position);
//		msg.SetRequesterVehicle(vehicle);
//		transmitter.BeginTransmission(msg);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Requests evacuation of the local player
//	//! \param[in] requesterID
//	//! \param[in] position
//	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
//	protected void RPC_RequestEvacuation(int requesterID, vector position)
//	{
//		if (GetTaskManager() && GetTaskManager().HasRequestedTask(requesterID))
//			return;
//		
//		SCR_CampaignNetworkComponent networkComponent = SCR_CampaignNetworkComponent.Cast(m_PlayerController.FindComponent(SCR_CampaignNetworkComponent));
//		if (!networkComponent)
//			return;
//
//		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(m_PlayerController.GetMainEntity());
//		if (!character)
//		   return;
//		
//		SCR_CampaignMilitaryBaseComponent closestBase = SCR_GameModeCampaign.GetInstance().GetBaseManager().FindClosestBase(character.GetOrigin());
//		if (closestBase)
//			networkComponent.SendPlayerMessage(SCR_ERadioMsg.REQUEST_EVAC, closestBase.GetCallsign(), checkHQReached: true);
//		
//		SCR_CampaignFaction requesterFaction = SCR_CampaignFaction.Cast(character.GetFaction());
//		if (!requesterFaction)
//		   return;
//
//		BaseRadioComponent radioComp = GetRadioComponent(m_PlayerController, character);
//		if (!radioComp)
//			return;
//		
//		BaseTransceiver transmitter = radioComp.GetTransceiver(0);
//		if (!transmitter)
//			return;
//		
//		// send message with requester faction and his ID
//		SCR_RequestEvacuationMessage msg = new SCR_RequestEvacuationMessage();
//		msg.SetRequesterMainBase(requesterFaction);
//		msg.SetRequesterID(requesterID);
//		msg.SetPosition(position);
//		transmitter.BeginTransmission(msg);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	//! \param[in] playerController
//	//! \param[in] character
//	//! \return
//	BaseRadioComponent GetRadioComponent(PlayerController playerController, ChimeraCharacter character)
//	{
//		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast(character.FindComponent(SCR_GadgetManagerComponent));
//		if (!gadgetManager)
//			return null;
//		
//		GenericEntity radio = GenericEntity.Cast(gadgetManager.GetGadgetByType(EGadgetType.RADIO));
//		if (!radio)
//			return null;
//		
//		BaseRadioComponent radioComp = BaseRadioComponent.Cast(radio.FindComponent(BaseRadioComponent));
//		if (!radioComp)
//			return null;
//		
//		return radioComp;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! \return
//	int GetPlayerId()
//	{
//		int playerID = m_PlayerController.GetPlayerId();
//		return playerID;
//	}
//
//	//------------------------------------------------------------------------------------------------
//	// constructor
//	//! \param[in] src
//	//! \param[in] ent
//	//! \param[in] parent
//	void SCR_CampaignTaskNetworkComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
//	{
//	}
//}
