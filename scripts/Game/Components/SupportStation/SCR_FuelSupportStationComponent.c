[ComponentEditorProps(category: "GameScripted/SupportStation", description: "")]
class SCR_FuelSupportStationComponentClass : SCR_BaseSupportStationComponentClass
{
	[Attribute(desc: "Sound effect played when fuel is updated. Broadcast to players. Leave empty if no sfx", category: "Fuel Support Station")]
	protected string m_sOnUpdateSoundEffectEventName;
	
	protected ref SCR_AudioSourceConfiguration m_OnUpdateAudioSourceConfiguration;
	
	//------------------------------------------------------------------------------------------------
	//! Get Audio config to play
	//! Will create it if it not yet exists. Returns null if no SoundFile or SoundEvent is set
	//! \return Sound Config
	SCR_AudioSourceConfiguration GetOnUpdateAudioConfig()
	{		
		//~ Create Audio source if it does not yet exist
		if (!m_OnUpdateAudioSourceConfiguration)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sOnUseSoundEffectFile) || SCR_StringHelper.IsEmptyOrWhiteSpace(m_sOnUpdateSoundEffectEventName))
				return null;
			
			m_OnUpdateAudioSourceConfiguration = new SCR_AudioSourceConfiguration();
			m_OnUpdateAudioSourceConfiguration.m_sSoundProject = m_sOnUseSoundEffectFile;
			m_OnUpdateAudioSourceConfiguration.m_sSoundEventName = m_sOnUpdateSoundEffectEventName;
			
			if (!CanMoveWithPhysics())
				m_OnUpdateAudioSourceConfiguration.m_eFlags = SCR_Enum.SetFlag(m_OnUpdateAudioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.Static);
			else 
				m_OnUpdateAudioSourceConfiguration.m_eFlags = SCR_Enum.RemoveFlag(m_OnUpdateAudioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.Static);
		}
		
		return m_OnUpdateAudioSourceConfiguration;
	}
}

class SCR_FuelSupportStationComponent : SCR_BaseSupportStationComponent
{		
	[Attribute(defvalue: EFuelFlowCapacityOut.FUEL_CARGO.ToString(), uiwidget: UIWidgets.SearchComboBox, desc: "Maximum Flow Capacity (per minute). Is only used if entity does not have a Fuelmanager with Nodes as scripted fuel nodes have the max flow assigned", enums: ParamEnumArray.FromEnum( EFuelFlowCapacityOut ), category: "Fuel Support Station Only")]
	protected EFuelFlowCapacityOut m_BackupMaxFlowCapacity;
	
	[Attribute(ESupportStationReasonInvalid.NO_FUEL_TO_GIVE.ToString(), desc: "The reason to display when the fuelManager has no fuel to give. For fuel canister this is diffrent then for example a tanker", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ESupportStationReasonInvalid), category: "Fuel Support Station Only")]
	protected ESupportStationReasonInvalid m_EmptyInvalidReason;
	
	protected SCR_FuelManagerComponent m_SupportStationFuelManager;
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		m_SupportStationFuelManager = SCR_FuelManagerComponent.Cast(owner.FindComponent(SCR_FuelManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action, vector actionPosition, out ESupportStationReasonInvalid reasonInvalid, out int supplyAmount)
	{
		if (!super.IsValid(actionOwner, actionUser, action, actionPosition, reasonInvalid, supplyAmount))
			return false;
		
		//~ Has no fuel to provide so not valid
		if (m_SupportStationFuelManager && !m_SupportStationFuelManager.HasFuelToProvide())
		{
			reasonInvalid = m_EmptyInvalidReason;
			return false;
		}
	
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.FUEL;
	}

	//------------------------------------------------------------------------------------------------
	//~  When fuel is removed and how much and from which node (Server only)
	protected void OnFuelAddedToVehicleServer(float fuelAmount, SCR_FuelNode nodeUsedForRefuel)
	{
		//~ No node was used as provider
		if (!nodeUsedForRefuel)
			return;
		
		//~ Update fuel node amount
		nodeUsedForRefuel.SetFuel(Math.Clamp(nodeUsedForRefuel.GetFuel() - fuelAmount, 0, nodeUsedForRefuel.GetMaxFuel()));
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get the max flow and potentially the node used for refueling
	protected void GetFuelNodeInfo(out float maxFlowCapacity, out SCR_FuelNode nodeUsedForRefuel, float actionDuration)
	{
		maxFlowCapacity = 0;
		nodeUsedForRefuel = null;
		
		//~ No fuel manager so use backup and always allow for fuel to be used
		if (!m_SupportStationFuelManager)
		{
			maxFlowCapacity = (m_BackupMaxFlowCapacity / 60) * actionDuration;
			return;
		}
			
		array<BaseFuelNode> nodes = {};
		m_SupportStationFuelManager.GetFuelNodesList(nodes);
		
		SCR_FuelNode scrNode;
		
		//~ Loop through all nodes and grab the first provider that has fuel
		foreach(BaseFuelNode node: nodes)
		{
			//~ Requires SCR_FuelNode
			scrNode = SCR_FuelNode.Cast(node);
			if (!scrNode)
				continue;
			
			//~ Can provide and has fuel
			if (scrNode.CanProvideFuel() && scrNode.GetFuel() > 0)
			{
				nodeUsedForRefuel = scrNode;
				
				maxFlowCapacity = (scrNode.GetMaxFlowCapacityOut() / 60) * actionDuration;
				
				//~ Fuel tank is almost empty so use current amount as max flow
				if (scrNode.GetFuel() < maxFlowCapacity)
					maxFlowCapacity = scrNode.GetFuel();
					
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Fuel does not execute super.OnExecutedServer as it has an on partly done
	override void OnExecutedServer(notnull IEntity actionOwner, notnull IEntity actionUser, notnull SCR_BaseUseSupportStationAction action)
	{	
		if (!actionOwner)
			return;
		
		//~ Get refuel action
		SCR_RefuelAtSupportStationAction refuelAction = SCR_RefuelAtSupportStationAction.Cast(action);
		if (!refuelAction)
		{
			Debug.Error2("SCR_FuelSupportStationComponent", "Was initiated by an user action that does not inherit from 'SCR_RefuelAtSupportStationAction'");
			return;
		}
		
		array<int> fuelTankIDs = {};
		refuelAction.GetFuelTankIDs(fuelTankIDs);
		
		//~ Target needs fuel manager and can be refueled
		SCR_FuelManagerComponent targetFuelManager = SCR_FuelManagerComponent.Cast(actionOwner.FindComponent(SCR_FuelManagerComponent));
		if (!targetFuelManager || !targetFuelManager.CanBeRefueledScripted(fuelTankIDs))
			return;
		
		//~ Get action duration per second
		float actionDuration = 1;
		if (action)
		{
			actionDuration = action.GetActionDuration();
			
			if (actionDuration < 0)
				actionDuration *= -1;
		}
		
		float maxFlowCapacityOut;
		SCR_FuelNode nodeUsedForRefuel;
		GetFuelNodeInfo(maxFlowCapacityOut, nodeUsedForRefuel, actionDuration);
		
		//~ Safty should never be called as HasFuelToProvide should catch this
		if (maxFlowCapacityOut <= 0)
			return;
		
		array<BaseFuelNode> nodes = {};
		targetFuelManager.GetFuelNodesList(nodes);
		
		float fuelToAdded = maxFlowCapacityOut;
		float preChangeFuel;
			
		SCR_FuelNode scrNode;
		
		bool maxFlowCapacityReached = false;
		float maxFlowNode = 1;
		
		//Refuel one node at the time
		for (int i = 0, count = nodes.Count(); i < count; i++)
		{			
			//~ Node already max fuel or null so ignore
			if (!nodes[i] || nodes[i].GetFuel() >= nodes[i].GetMaxFuel())
				continue;
			
			scrNode = SCR_FuelNode.Cast(nodes[i]);
			
			//~ Check if action allows the node to be filled
			if (refuelAction && !refuelAction.CheckIfFuelNodeIsValid(scrNode))
				continue;
			
			//~ Fuel before change
			preChangeFuel = nodes[i].GetFuel();
			
			if (scrNode)
			{
				//~ Node cannot receive fuel
				if (!scrNode.CanReceiveFuel())
					continue;

				maxFlowNode = (scrNode.GetMaxFlowCapacityIn() / 60) * actionDuration;
				
				//~ Check if the max flow capacity is reached (With multiplier)
				maxFlowCapacityReached = fuelToAdded > maxFlowNode;
				
				//~ Make sure node receives only what max flow capacity allows (with multiplier)
				scrNode.SetFuel(Math.Clamp(scrNode.GetFuel() + Math.Clamp(fuelToAdded, 0, maxFlowNode), 0, scrNode.GetMaxFuel()));
			}
			//~ Simply add to node as there is no max flow capacity
			else 
			{
				nodes[i].SetFuel(Math.Clamp(nodes[i].GetFuel() + fuelToAdded, 0, nodes[i].GetMaxFuel()));
			}
			
			fuelToAdded -= nodes[i].GetFuel() - preChangeFuel;				
		
			if (maxFlowCapacityReached)
				break;
			
			if (fuelToAdded <= 0)
			{
				fuelToAdded = 0;
				break;
			}
		}
		
		//~ Sends over amount of fuel that was added to vehicle. In case supplies or fuel needs to be removed
		OnFuelAddedToVehicleServer(maxFlowCapacityOut - fuelToAdded, nodeUsedForRefuel);
		
		//~ Fuel uses own function to broadcast to include current percentage so super.OnExecutedServer() is never called
		float fuelTankFillPercentage = targetFuelManager.GetTotalFuel() / targetFuelManager.GetTotalMaxFuel();
		bool currentTankFull = true;
		
		//~ Check if current tank is full
		foreach (BaseFuelNode node : nodes)
		{
			if (!refuelAction.CheckIfFuelNodeIsValid(SCR_FuelNode.Cast(node)))
				continue;
			
			//~ Current tank not yet full
			if (node.GetFuel() < node.GetMaxFuel())
			{
				currentTankFull = false;
				break;
			}
		}

		RplId ownerId;
		RplId userId;
		int playerId;

		//~ Get broadcast ids
		FindEntityIds(actionOwner, actionUser, ownerId, userId, playerId);
		
		//~ On fuel executed
		OnExecuteFuel(actionOwner, actionUser, playerId, fuelTankFillPercentage, currentTankFull, action);
		Rpc(OnExecuteFuelBroadcast, ownerId, userId, playerId, fuelTankFillPercentage, currentTankFull, action.GetActionID());
	}
	
	//------------------------------------------------------------------------------------------------
	//~ called by Server to convert ReplicationIDs to entity and player
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnExecuteFuelBroadcast(RplId ownerId, RplId userId, int userPlayerId, float fuelTankFillPercentage, bool currentTankFull, int actionId)
	{
		IEntity actionOwner, actionUser;
		GetEntitiesFromID(ownerId, userId, actionOwner, actionUser);
		
		SCR_BaseUseSupportStationAction action;
		if (actionOwner)
		{
			ActionsManagerComponent actionManager = ActionsManagerComponent.Cast(actionOwner.FindComponent(ActionsManagerComponent));
			if (actionManager)
				action = SCR_BaseUseSupportStationAction.Cast(actionManager.FindAction(actionId));
			
			OnExecuteFuel(actionOwner, actionUser, userPlayerId, fuelTankFillPercentage, currentTankFull, action);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Called by fuel only on server and client
	protected void OnExecuteFuel(IEntity actionOwner, IEntity actionUser, int playerId, float fuelPercentage, bool currentTankFull, SCR_BaseUseSupportStationAction action)
	{		
		//~ On succesfully executed
		OnSuccessfullyExecuted(actionOwner, actionUser, action);
		
		//~ Refuel not done
		if (fuelPercentage < 1 && !currentTankFull)
		{
			PlayRefuelUpdateSound(actionOwner, action);	
		}
		//~ Refuel of current tank is done
		else if (fuelPercentage < 1 && currentTankFull)
		{
			//~ Refuel done sfx
			PlaySoundEffect(GetOnUseAudioConfig(), actionOwner, action);
		}
		//~ Refuel is fully done
		else 
		{
			//~ Refuel done sfx
			PlaySoundEffect(GetOnUseAudioConfig(), actionOwner, action);
			
			//~ Play full done voice event (if any)
			PlayCharacterVoiceEvent(actionUser);
		}
		
		//~ Do not send notification or no user
		if (!GetSendNotificationOnUse() || !actionUser)
			return;
		
		//~ Editable entity not found
		SCR_EditableEntityComponent userEditableEntity = SCR_EditableEntityComponent.Cast(actionUser.FindComponent(SCR_EditableEntityComponent));
		if (!userEditableEntity)
			return;
		
		RplId userRplId = Replication.FindId(userEditableEntity);
		
		//~ Check if player is in vehicle that is being refueled
		array<int> playersInVehicle = {};
		GetPlayerIdsInVehicle(actionOwner, playersInVehicle);
		
		//~ In vehicle so send notification that another player is refueling the vehicle you are in
		if (playersInVehicle.Contains(SCR_PlayerController.GetLocalPlayerId()))
		{
			if (fuelPercentage < 1 && !currentTankFull)
				SCR_NotificationsComponent.SendLocal(ENotification.SUPPORTSTATION_REFUELED_BY_OTHER_UPDATE, userRplId, fuelPercentage * 1000);
			else if (fuelPercentage < 1 && currentTankFull)
				SCR_NotificationsComponent.SendLocal(ENotification.SUPPORTSTATION_REFUELED_BY_OTHER_TANK_FULL, userRplId, fuelPercentage * 1000);
			else 
				SCR_NotificationsComponent.SendLocal(ENotification.SUPPORTSTATION_REFUELED_BY_OTHER_DONE, userRplId);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnExecute(IEntity actionOwner, IEntity actionUser, int playerId, SCR_BaseUseSupportStationAction action)
	{
		//~ Clear on execute so it does not call on complete
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void PlayRefuelUpdateSound(IEntity actionOwner, SCR_BaseUseSupportStationAction action)
	{
		SCR_FuelSupportStationComponentClass classData = SCR_FuelSupportStationComponentClass.Cast(GetComponentData(GetOwner()));
		if (!classData)
			return;
		
		//~  Play sound
		PlaySoundEffect(classData.GetOnUpdateAudioConfig(), actionOwner, action);
	}
}
