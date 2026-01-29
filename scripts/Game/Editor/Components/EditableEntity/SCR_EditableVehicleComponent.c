[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableVehicleComponentClass : SCR_EditableEntityComponentClass
{
}

//! @ingroup Editable_Entities

//! Special configuration for editable wehicle.
class SCR_EditableVehicleComponent : SCR_EditableEntityComponent
{
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;
	protected SCR_VehicleFactionAffiliationComponent m_VehicleFactionAffiliation;
	protected ref ScriptInvoker m_OnUIRefresh;

	protected int m_iPlayersInVehicle;

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetPlayerCountInVehicle()
	{
		return m_iPlayersInVehicle;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionUpdate(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		if (m_OnUIRefresh)
			m_OnUIRefresh.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDestroyed(IEntity owner)
	{
		if (m_OnUIRefresh)
			m_OnUIRefresh.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override Faction GetFaction()
	{
		//--- Destroyed entities have no faction
		if (IsDestroyed())
			return null;

		if (m_VehicleFactionAffiliation)
			return m_VehicleFactionAffiliation.GetAffiliatedFaction();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnUIRefresh()
	{
		if (!m_OnUIRefresh)
			m_OnUIRefresh = new ScriptInvoker();

		return m_OnUIRefresh;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent GetAIGroup()
	{
		if (!m_CompartmentManager)
			return null;

		SCR_EditableEntityComponent occupant;
		array<BaseCompartmentSlot> compartments = {};
		for (int i = 0, count =	m_CompartmentManager.GetCompartments(compartments); i < count; i++)
		{
			occupant = SCR_EditableEntityComponent.GetEditableEntity(compartments[i].GetOccupant());
			if (occupant)
				return occupant.GetAIGroup();
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent GetAIEntity()
	{
		if (!m_CompartmentManager)
			return null;

		SCR_EditableEntityComponent occupant;
		array<BaseCompartmentSlot> compartments = {};
		for (int i = 0, count =	m_CompartmentManager.GetCompartments(compartments); i < count; i++)
		{
			occupant = SCR_EditableEntityComponent.GetEditableEntity(compartments[i].GetOccupant());
			if (occupant)
				return occupant;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	override int GetPlayerID()
	{
		if (!m_CompartmentManager)
			return 0;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		SCR_EditableEntityComponent occupant;
		array<BaseCompartmentSlot> compartments = {};
		int playerIdToReturn = 0;
		for (int i = 0, count =	m_CompartmentManager.GetCompartments(compartments); i < count; i++)
		{
			BaseCompartmentSlot compartment = compartments[i];
			occupant = SCR_EditableEntityComponent.GetEditableEntity(compartment.GetOccupant());
			if (occupant)
			{
				int playerId = occupant.GetPlayerID();
				if (playerId <= 0)
					continue;

				if (playerIdToReturn == 0)
					playerIdToReturn = playerId;
				else if (compartment.GetType() == ECompartmentType.PILOT)
					return playerId;
			}
		}

		return playerIdToReturn;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void UpdatePlayerCountInVehicle()
	{
		if (!m_CompartmentManager)
			return;

		SCR_EditableEntityComponent occupant;
		array<BaseCompartmentSlot> compartments = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();

		int playerCount = 0;
		for (int i = 0, count =	m_CompartmentManager.GetCompartments(compartments); i < count; i++)
		{
			occupant = SCR_EditableEntityComponent.GetEditableEntity(compartments[i].GetOccupant());
			if (occupant)
			{
				int playerId = playerManager.GetPlayerIdFromControlledEntity(occupant.GetOwner());
				if (playerId != 0)
					playerCount++;
			}
		}

		m_iPlayersInVehicle = playerCount;
		if (m_OnUIRefresh)
			m_OnUIRefresh.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override int GetCrew(out notnull array<CompartmentAccessComponent> crewCompartmentAccess, bool ignorePlayers = true)
	{
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return 0;

		PlayerManager playerManager;

		//If Ignore players
		if (ignorePlayers)
			playerManager = GetGame().GetPlayerManager();

		array<IEntity> occupants = {};
		compartmentManager.GetOccupants(occupants);
		CompartmentAccessComponent compartmentAccess;

		foreach (IEntity occupant : occupants)
		{
			if (ignorePlayers && playerManager.GetPlayerIdFromControlledEntity(occupant) > 0)
				continue;

			compartmentAccess = CompartmentAccessComponent.Cast(occupant.FindComponent(CompartmentAccessComponent));

			if (compartmentAccess && compartmentAccess.IsInCompartment())
				crewCompartmentAccess.Insert(compartmentAccess);
		}

		return crewCompartmentAccess.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Add feedback to players that they are teleported when inside of the vehicle
	protected void PlayerTeleportedFeedback()
	{
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;

		array<IEntity> occupants = {};
		SCR_EditableCharacterComponent editableCharacter;
		compartmentManager.GetOccupants(occupants);

		foreach (IEntity occupant : occupants)
		{
			editableCharacter = SCR_EditableCharacterComponent.Cast(occupant.FindComponent(SCR_EditableCharacterComponent));
			if (editableCharacter)
				editableCharacter.PlayerTeleportedByParentFeedback(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void SetTransform(vector transform[4], bool changedByUser = false)
	{
		super.SetTransform(transform, changedByUser);

		//~ Add feedback to players that they are teleported when inside of the vehicle
		PlayerTeleportedFeedback();
	}

	//--------------------------------------------------- Spawn occupants ---------------------------------------------------\\

	//------------------------------------------------------------------------------------------------
	//! Check if vehicle can spawn characters (Of vehicle Faction) in the vehicle. Cannot spawn characters in vehicle if occupied by another faction that is hostile
	//! \param[in] compartmentTypes Given compartment types that need to be checked if can be filled.
	//! \param[in] checkHasDefaultOccupantsData
	//! \param[in] playerID If server player ID must be given of the player that wans to check if can occupy, if client no ID needs to be given
	//! \param[in] checkEditorBudget If true will check if characters to spawn are within budget. Function will return false if there isn't enough budget
	//! \param[in] checkOccupyingFaction If true function will check if the faction of the vehicle is null (empty) or the same/friendly to the static faction as given in the editableUIInfo. Function will return false if faction is hostile or neutral
	//! \param[in] checkForFreeCompartments If true will check if there are compartments free for the given compartments types. Function will return false if there are no free compartments of given type
	//! \param[in] checkForFreeDefaultCompartments
	//! \return Will return true if a character can be spawned in at least one compartment
	bool CanOccupyVehicleWithCharacters(array<ECompartmentType> compartmentTypes, bool checkHasDefaultOccupantsData, int playerID, bool checkEditorBudget = true, bool checkOccupyingFaction = true, bool checkForFreeCompartments = true, bool checkForFreeDefaultCompartments = false)
	{
		FactionKey factionKey = string.Empty;

		if (!m_CompartmentManager)
			return false;

		if (checkOccupyingFaction)
		{
			SCR_EditableEntityUIInfo uiInfo = SCR_EditableEntityUIInfo.Cast(GetInfo());
			if (uiInfo)
				factionKey = uiInfo.GetFactionKey();
		}

		bool hasEnoughBudgetForDefaultCompartments;
		bool hasFreeDefaultCompartments;

		if (checkEditorBudget || checkForFreeDefaultCompartments)
			hasEnoughBudgetForDefaultCompartments = HasEnoughBudgetForDefaultOccupants(compartmentTypes, playerID, EEditorMode.EDIT, hasFreeDefaultCompartments);

		if (checkEditorBudget && !hasEnoughBudgetForDefaultCompartments)
			return false;

		if (checkForFreeDefaultCompartments && !hasFreeDefaultCompartments)
			return false;

		return m_CompartmentManager.CanOccupy(compartmentTypes, checkHasDefaultOccupantsData, factionKey, checkOccupyingFaction, checkForFreeCompartments);
	}

	//------------------------------------------------------------------------------------------------
	//~ Check if there is enough budget to spawn default occupants
	protected bool HasEnoughBudgetForDefaultOccupants(array<ECompartmentType> compartmentTypes, int playerID, EEditorMode editorMode, out bool noFreeDefaultCompartments)
	{
		noFreeDefaultCompartments = true;

		array<BaseCompartmentSlot> compartments = {};

		//~ Get all free compartments of given types
		foreach (ECompartmentType compartmentType : compartmentTypes)
		{
			m_CompartmentManager.GetFreeCompartmentsOfType(compartments, compartmentType);
		}

		array<ResourceName> occupantsToSpawn = {};
		ResourceName occupant;

		foreach (BaseCompartmentSlot compartment : compartments)
		{
			occupant = compartment.GetDefaultOccupantPrefab();
			if (!occupant.IsEmpty())
				occupantsToSpawn.Insert(occupant);
		}

		//~ Is empty so it has enough budget but will also return that there are no free seats
		if (occupantsToSpawn.IsEmpty())
		{
			noFreeDefaultCompartments = false;
			return true;
		}

		SCR_EditorManagerEntity editorManager;
		
		//~ Get EditorManager from given player
		if (playerID > 0)
		{
			SCR_EditorManagerCore editorCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (!editorCore)
				return false;
			
			editorManager = editorCore.GetEditorManager(playerID);
		}
		//~ Get Editor manager from local player
		else 
		{
			editorManager = SCR_EditorManagerEntity.GetInstance();
		}
		
		if (!editorManager)
			return false;
		
		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(editorMode);
		if (!modeEntity)
			return false;

		SCR_PlacingEditorComponent placingComponent;

		placingComponent = SCR_PlacingEditorComponent.Cast(modeEntity.FindComponent(SCR_PlacingEditorComponent));
		return placingComponent.IsThereEnoughBudgetToSpawnVehicleOccupants(occupantsToSpawn);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn characters (Of vehicle Faction) within the Vehicle (Server only)
	//! \param[in] compartmentTypes Given compartment types that need to be filled with characters
	void OccupyVehicleWithDefaultCharacters(notnull array<ECompartmentType> compartmentTypes)
	{
		m_CompartmentManager.SpawnDefaultOccupants(compartmentTypes);
	}

	//------------------------------------------------------------------------------------------------
	//! Get Compartment manager of vehicle
	//! \return Compartment manager
	SCR_BaseCompartmentManagerComponent GetCompartmentManager()
	{
		return m_CompartmentManager;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent EOnEditorPlace(out SCR_EditableEntityComponent parent, SCR_EditableEntityComponent recipient, EEditorPlacingFlags flags, bool isQueue, int playerID = 0)
	{
		//~ No vehicle placing flag
		if (!SCR_Enum.HasPartialFlag(flags, EEditorPlacingFlags.VEHICLE_CREWED | EEditorPlacingFlags.VEHICLE_PASSENGER))
			return this;

		//~ Todo: Check if EEditorPlacingFlags.CHARACTER_PLAYER and spawn the first character as player. If Only EEditorPlacingFlags.CHARACTER_PLAYER add itself as pilot (or any other availible slots)

		SCR_EditableVehicleUIInfo uiInfo = SCR_EditableVehicleUIInfo.Cast(GetInfo());
		array<ECompartmentType> compartmentsToFill = {};

		bool hasFreeSeats;

		//~ Check crew budget
		if (SCR_Enum.HasFlag(flags, EEditorPlacingFlags.VEHICLE_CREWED) && HasEnoughBudgetForDefaultOccupants(SCR_BaseCompartmentManagerComponent.CREW_COMPARTMENT_TYPES, playerID, EEditorMode.EDIT, hasFreeSeats))
		{
			//~ Occupy with Crew
			if (!uiInfo || !uiInfo.GetEditorPlaceAsOneGroup())
				OccupyVehicleWithDefaultCharacters(SCR_BaseCompartmentManagerComponent.CREW_COMPARTMENT_TYPES);
			else
				compartmentsToFill.InsertAll(SCR_BaseCompartmentManagerComponent.CREW_COMPARTMENT_TYPES);
		}

		//~ Check passenger budget
		if (SCR_Enum.HasFlag(flags, EEditorPlacingFlags.VEHICLE_PASSENGER) && HasEnoughBudgetForDefaultOccupants(SCR_BaseCompartmentManagerComponent.PASSENGER_COMPARTMENT_TYPES, playerID, EEditorMode.EDIT, hasFreeSeats))
		{
			//~ Occupy with Passengers
			if (!uiInfo || !uiInfo.GetEditorPlaceAsOneGroup())
				OccupyVehicleWithDefaultCharacters(SCR_BaseCompartmentManagerComponent.PASSENGER_COMPARTMENT_TYPES);
			else
				compartmentsToFill.InsertAll(SCR_BaseCompartmentManagerComponent.PASSENGER_COMPARTMENT_TYPES);
		}

		//~ Occupy vehicle with both Crew and Passengers in one group (If both placing flags where selected)
		if (!compartmentsToFill.IsEmpty())
			OccupyVehicleWithDefaultCharacters(compartmentsToFill);
		
		return this;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		//--- Move to root when the vehicle is activated
		if (activeState)
			SetParentEntity(null);
		//ClearEventMask(m_Owner, EntityEvent.PHYSICSACTIVE);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_DISABLE))
			return;

		super.OnPostInit(owner);

		//SetEventMask(owner, EntityEvent.PHYSICSACTIVE);

		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
		m_VehicleFactionAffiliation = SCR_VehicleFactionAffiliationComponent.Cast(owner.FindComponent(SCR_VehicleFactionAffiliationComponent));
		if (m_VehicleFactionAffiliation)
			m_VehicleFactionAffiliation.GetOnFactionChanged().Insert(OnFactionUpdate);

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
			eventHandlerManager.RegisterScriptHandler("OnDestroyed", owner, OnDestroyed);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_EditableVehicleComponent()
	{
		if (m_VehicleFactionAffiliation)
			m_VehicleFactionAffiliation.GetOnFactionChanged().Remove(OnFactionUpdate);
	}
}
