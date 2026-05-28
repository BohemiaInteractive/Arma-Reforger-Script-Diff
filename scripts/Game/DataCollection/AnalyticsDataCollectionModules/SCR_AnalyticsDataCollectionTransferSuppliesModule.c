enum SCR_EAnalyticalStorageType
{
	CHARACTER,
	ORPHAN,
	BASE,
	HELICOPTER,
	WHEELED_VEHICLE
}

class SCR_TransferSuppliesInteraction
{
	ResourceName m_sFromPrefab;
	ResourceName m_sToPrefab;
	string m_sFromType;
	string m_sToType;
	string m_sFromPosition;
	string m_sToPosition
	string m_sPlayerId;
	string m_sFaction;
	float m_fValue;
	
	// constructor
	//! \param[in] from
	//! \param[in] to
	//! \param[in] fromType
	//! \param[in] toType
	//! \param[in] value
	//! \param[in] isPlayer
	void SCR_TransferSuppliesInteraction(ResourceName from, ResourceName to, string fromType, string toType, float value, string fromPos, string toPos, string playerId, string faction)
	{
		m_sFromPrefab = from;
		m_sToPrefab = to;
		m_sFromType = fromType;
		m_sToType = toType;
		m_sFromPosition = fromPos;
		m_sToPosition = toPos;
		m_sPlayerId = playerId;
		m_fValue = value;
		m_sFaction = faction;
	}
}

[BaseContainerProps()]
class SCR_AnalyticsDataCollectionTransferSuppliesModule : SCR_AnalyticsDataCollectionModule
{
	protected ref array<ref SCR_TransferSuppliesInteraction> m_Interactions = new array<ref SCR_TransferSuppliesInteraction>();
	protected static const string BASE = "BASE";
	// SessionMeasures fires every 900 s; cap prevents unbounded growth between flushes
	protected static const int MAX_INTERACTIONS = 2000;
	
	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		SCR_CampaignNetworkComponent.GetOnTransferSupplies().Insert(OnTransferSuppliesByPlayer);
		SCR_SuppliesTransferWaypointState.GetOnAITransferedSupplies().Insert(OnTransferSuppliesByAI);
		super.Enable();
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		SCR_CampaignNetworkComponent.GetOnTransferSupplies().Remove(OnTransferSuppliesByPlayer);
		SCR_SuppliesTransferWaypointState.GetOnAITransferedSupplies().Remove(OnTransferSuppliesByAI);
		super.Disable();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeEnd()
	{
		// Game ends and so we need to record the latest data which will be send alongside missionEnd event
		SessionMeasures();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the type of consumer/generator by the resourceComponent
	//! \param[in] resourceComponent
	protected string GetTypeFromResourceComponent(SCR_ResourceComponent resourceComponent)
	{
		IEntity parent = resourceComponent.GetOwner().GetRootParent();
		
		// Check first by Entity type
		if (ChimeraCharacter.Cast(parent))
			return typename.EnumToString(SCR_EAnalyticalStorageType, SCR_EAnalyticalStorageType.CHARACTER);

		Vehicle vehicle = Vehicle.Cast(parent);
		if (!vehicle)
		{	
			// Once the other entities are discarded, check if the component comes from a base or is orphan
			// True means it is a BASE
			if(DistinguishOrphanAndBase(resourceComponent, parent))
				return typename.EnumToString(SCR_EAnalyticalStorageType, SCR_EAnalyticalStorageType.BASE);
			
			return typename.EnumToString(SCR_EAnalyticalStorageType, SCR_EAnalyticalStorageType.ORPHAN);
		}

		if (SCR_HelicopterControllerComponent.Cast(vehicle.GetVehicleController()))
			return typename.EnumToString(SCR_EAnalyticalStorageType, SCR_EAnalyticalStorageType.HELICOPTER);
		
		return typename.EnumToString(EVehicleType, vehicle.m_eVehicleType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the resourceComponents belongs to an Orphan or a base
	//! \param[in] resourceComponent
	//! \param[in] parent
	protected bool DistinguishOrphanAndBase(SCR_ResourceComponent resourceComponent, IEntity parent)
	{
		SCR_ResourceContainer containerToCheck = resourceComponent.GetContainer(EResourceType.SUPPLIES);
		bool areContainersOrphans = containerToCheck && containerToCheck.GetStorageType() == EResourceContainerStorageType.ORPHAN;
		bool missingInventoryComponent = !parent || (parent && !parent.FindComponent(InventoryItemComponent));
		
		if (areContainersOrphans || missingInventoryComponent)
			return false; // It is an ORPHAN
		
		return true; // It is a BASE
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] type
	//! \param[in] resource
	//! \param[in] playerController
	//! \param[in] utility
	//! \return
	string GetDestinationPosition(string type, SCR_ResourceComponent resource, PlayerController playerController, SCR_AIGroupUtilityComponent utility)
	{
		string ownerPos = resource.GetOwner().GetOrigin().ToString();
		IEntity resourceOwner = resource.GetOwner();
		if (!resourceOwner)
			return ownerPos;

		if (type != BASE)
		{
			if (!playerController)
				return utility.GetAIAgent().GetOrigin().ToString();
			
			PlayerId transferingPlayerId = playerController.GetPlayerId();

			IEntity playerEntity = SCR_PossessingManagerComponent.GetPlayerMainEntity(transferingPlayerId);
			if (playerEntity)
				return playerEntity.GetOrigin().ToString();
		}

		SCR_CampaignMilitaryBaseComponent base;
		if (!GetMilitaryBase(resourceOwner, base))
			return ownerPos;

		if (!base.GetOwner())
			return ownerPos;

		return base.GetOwner().GetOrigin().ToString();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resourceOwner
	//! \param[out] outBase
	//! \return
	bool GetMilitaryBase(IEntity resourceOwner, out SCR_CampaignMilitaryBaseComponent outBase)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return false;

		SCR_CampaignMilitaryBaseManager manager = campaign.GetBaseManager();
		if (!manager)
			return false;

		SCR_CampaignMilitaryBaseComponent base = manager.FindClosestBase(resourceOwner.GetOrigin());
		if (!base)
			return false;

		outBase = base;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerController
	//! \return
	string GetTransferingPlayerFaction(PlayerController playerController)
	{
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (!controlledEntity)
			return "";

		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
			return factionAffiliation.GetAffiliatedFaction().GetFactionKey();
		else
			return "";
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] utility
	//! \return
	string GetTransferingAIFaction(SCR_AIGroupUtilityComponent utility)
	{
		SCR_AIGroup aiGroup = utility.m_Owner;
		if (!aiGroup)
			return "";

		return aiGroup.GetFaction().GetFactionKey();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerController
	//! \return
	string GetCharacterUniqueId(notnull PlayerController playerController)
	{
		PlayerId playerid = playerController.GetPlayerId();

		return SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerid);
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName GetPrefabName(SCR_ResourceComponent resourceComponent)
	{
		return resourceComponent.GetOwner().GetRootParent().GetPrefabData().GetPrefabName();
	}
	
	//------------------------------------------------------------------------------------------------
	protected ResourceName GetPrefabNameFromAITruck(SCR_AIGroupUtilityComponent utility)
	{
		AIGroup group = AIGroup.Cast(utility.GetAIAgent());
		if (!group)
			return "";

		AIAgent leaderAgent = group.GetLeaderAgent();
		if (!leaderAgent)
			return "";

		ChimeraCharacter character = ChimeraCharacter.Cast(leaderAgent.GetControlledEntity());
		if (!character)
			return "";

		CompartmentAccessComponent compartment = character.GetCompartmentAccessComponent();
		if (!compartment)
			return "";

		return compartment.GetVehicleIn(character).GetPrefabData().GetPrefabName();
	}
	
	//------------------------------------------------------------------------------------------------
	protected ResourceName SetPrefabName(bool isPlayer, string resourceType, SCR_ResourceComponent resourceComponent, SCR_AIGroupUtilityComponent utility)
	{
		if(!isPlayer && resourceType == "SUPPLY_TRUCK")
			return GetPrefabNameFromAITruck(utility);
		else
			return GetPrefabName(resourceComponent);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTransferSuppliesByPlayer(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue, PlayerController playerController)
	{
		OnTransferSupplies(interactionType, resourceComponentFrom, resourceComponentTo, resourceType, resourceValue, playerController, null);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTransferSuppliesByAI(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue, SCR_AIGroupUtilityComponent utility)
	{
		OnTransferSupplies(interactionType, resourceComponentFrom, resourceComponentTo, resourceType, resourceValue, null, utility);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTransferSupplies(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue, PlayerController playerController, SCR_AIGroupUtilityComponent utility)
	{
		if (resourceValue <= 0)
			return;

		if (m_Interactions.Count() >= MAX_INTERACTIONS)
		{
			Print("SCR_AnalyticsDataCollectionTransferSuppliesModule: interaction cap reached, dropping entry", LogLevel.WARNING);
			return;
		}

		bool isPlayer = playerController;

		ResourceName fromPrefab;
		string fromType;
		string fromPos;
		if (interactionType == EResourcePlayerInteractionType.VEHICLE_LOAD)
		{
			fromPrefab = "-";
			fromType = BASE;
			fromPos = GetMilitaryBasePosition(resourceComponentTo);
		}
		else
		{
			fromType = GetPrefabType(resourceComponentFrom, isPlayer);
			fromPos = GetDestinationPosition(fromType, resourceComponentFrom, playerController, utility);
			fromPrefab = SetPrefabName(isPlayer, fromType, resourceComponentFrom, utility);
		}
		
		ResourceName toPrefab;
		string toType;
		string toPos;
		if (interactionType == EResourcePlayerInteractionType.VEHICLE_UNLOAD)
		{
			toPrefab = "-";
			toType = BASE;
			toPos = GetMilitaryBasePosition(resourceComponentFrom);
		}
		else
		{
			toType = GetPrefabType(resourceComponentTo, isPlayer);
			toPos = GetDestinationPosition(toType, resourceComponentTo, playerController, utility);
			toPrefab = SetPrefabName(isPlayer, toType, resourceComponentTo, utility);
		}

		string tranferFactionName;
		if (isPlayer)
			tranferFactionName = GetTransferingPlayerFaction(playerController);
		else
			tranferFactionName = GetTransferingAIFaction(utility);

		string characterName;
		if (isPlayer)
			characterName = GetCharacterUniqueId(playerController);
		else
			characterName = "AI";

		PrintFormat("SCR_AnalyticsDataCollectionTransferSuppliesModule: Transferring %1 supplies from %2 (%3) to %4 (%5) \n Faction is: %6 , and it is moved from %7 to %8", resourceValue, fromPrefab, fromType, toPrefab, toType, tranferFactionName, fromPos, toPos, level: LogLevel.DEBUG);
		m_Interactions.Insert(new SCR_TransferSuppliesInteraction(fromPrefab, toPrefab, fromType, toType, resourceValue, fromPos, toPos, characterName, tranferFactionName));
	}	

	//------------------------------------------------------------------------------------------------
	protected string GetPrefabType(SCR_ResourceComponent resourceComponent, bool isPlayer)
	{
		string type = GetTypeFromResourceComponent(resourceComponent);

		if (!isPlayer && type != BASE)
			type = "SUPPLY_TRUCK";

		return type;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetMilitaryBasePosition(SCR_ResourceComponent resourceComponent)
	{
		string position;
		SCR_CampaignMilitaryBaseComponent base;

		if (GetMilitaryBase(resourceComponent.GetOwner(), base))
			position = base.GetOwner().GetOrigin().ToString();

		return position;
	}

	//------------------------------------------------------------------------------------------------
	override void SessionMeasures()
	{
		if (!m_bIsEnabled)
			return;

		SCR_SessionDataEvent sessionDataEvent = GetSessionDataEvent();
		if (!sessionDataEvent)
			return;

		sessionDataEvent.array_supplies_from_prefab = "[";
		sessionDataEvent.array_supplies_to_prefab	= "[";
		sessionDataEvent.array_supplies_from_type 	= "[";
		sessionDataEvent.array_supplies_to_type 	= "[";
		sessionDataEvent.array_supplies_values		= "[";
		sessionDataEvent.array_supplies_by_player = "[";
		sessionDataEvent.array_supplies_from_pos = "[";
		sessionDataEvent.array_supplies_to_pos = "[";
		sessionDataEvent.array_supplies_faction = "[";

		foreach (int i, SCR_TransferSuppliesInteraction interaction : m_Interactions)
		{
			if (i != 0)
			{
				sessionDataEvent.array_supplies_from_prefab += ",";
				sessionDataEvent.array_supplies_to_prefab 	+= ",";
				sessionDataEvent.array_supplies_from_type 	+= ",";
				sessionDataEvent.array_supplies_to_type 	+= ",";
				sessionDataEvent.array_supplies_values 		+= ",";
				sessionDataEvent.array_supplies_by_player += ",";
				sessionDataEvent.array_supplies_from_pos += ",";
				sessionDataEvent.array_supplies_to_pos += ",";
				sessionDataEvent.array_supplies_faction += ",";
			}
			
			sessionDataEvent.array_supplies_from_prefab += "\"" + interaction.m_sFromPrefab + "\"";
			sessionDataEvent.array_supplies_to_prefab 	+= "\"" + interaction.m_sToPrefab + "\"";
			sessionDataEvent.array_supplies_from_type 	+= "\"" + interaction.m_sFromType + "\"";
			sessionDataEvent.array_supplies_to_type 	+= "\"" + interaction.m_sToType + "\"";
			sessionDataEvent.array_supplies_values 		+= "\"" + interaction.m_fValue.ToString() + "\"";
			sessionDataEvent.array_supplies_by_player 	+= "\"" + interaction.m_sPlayerId + "\"";
			sessionDataEvent.array_supplies_from_pos += "\"" + interaction.m_sFromPosition + "\"";
			sessionDataEvent.array_supplies_to_pos += "\"" + interaction.m_sToPosition + "\"";
			sessionDataEvent.array_supplies_faction += "\"" + interaction.m_sFaction + "\"";
		}
		
		sessionDataEvent.array_supplies_from_prefab += "]";
		sessionDataEvent.array_supplies_to_prefab 	+= "]";
		sessionDataEvent.array_supplies_from_type 	+= "]";
		sessionDataEvent.array_supplies_to_type 	+= "]";
		sessionDataEvent.array_supplies_values 		+= "]";
		sessionDataEvent.array_supplies_by_player += "]";
		sessionDataEvent.array_supplies_from_pos += "]";
		sessionDataEvent.array_supplies_to_pos += "]";
		sessionDataEvent.array_supplies_faction += "]";
		
		m_Interactions.Clear();
	}
}
