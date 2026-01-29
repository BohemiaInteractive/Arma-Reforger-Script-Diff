[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableCharacterComponentClass : SCR_EditableEntityComponentClass
{
	[Attribute(category: "Character", params: "et")]
	private ResourceName m_PrefabGroup;

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetPrefabGroup()
	{
		return m_PrefabGroup;
	}
}

//! @ingroup Editable_Entities

//! Special configuration for editable character.
class SCR_EditableCharacterComponent : SCR_EditableEntityComponent
{
	protected AIAgent m_Agent;
	protected AIControlComponent m_AgentControlComponent;
	protected float m_fPlayerDrawDistance;
	protected ref ScriptInvoker m_OnUIRefresh = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnCharacterMovedInVehicle = new ScriptInvoker(); //~ Authority Only, Returns this character and vehicle character is placed in. Is NULL if placing failed
	protected ref ScriptInvokerEntity m_OnDestroyed;
	protected int m_inDeadPlayerID;
	
	[RplProp()]
	protected bool m_bIsPlayerPending;

	protected bool m_bShouldRecalculateDrawDistance;

	//! Authority only, Allows character to be forced into a specific vehicle position and will delete it if failed
	protected ref array<ECompartmentType> m_aForceVehicleCompartments;

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	void SetIsPlayerPending(int playerId)
	{
		m_bIsPlayerPending = playerId != 0;
		
		RplComponent rplComponent = GetRplComponent();
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
			Replication.BumpMe();
	}

	//:| This is a similar approach with SCR_ChimeraAIAgent's IsPlayerPending_S method.
	//:| The reason the similar behavior is also implemented here is that this approach allows us to extend the behavior within Editor if needed.
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsPlayerPending()
	{
		return m_bIsPlayerPending;
	}

	//------------------------------------------------------------------------------------------------
	//! Get AI agent of the character.
	//! \return AI agent on server, null on clients
	AIAgent GetAgent()
	{
		return m_Agent;
	}

	//------------------------------------------------------------------------------------------------
	//! Get event called when character is moved in the vehicle by GM and systems
	//! Called only on server.
	//! Invoker params are: this character, IEntity vehicle. This is null if moving in vehicle failed
	//! \return Script invoker
	ScriptInvoker GetOnCharacterMovedInVehicle()
	{
		return Event_OnCharacterMovedInVehicle;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get event called when character destroyed
	//! \return Script invoker
	ScriptInvokerEntity GetOnDestroyed()
	{
		if (!m_OnDestroyed)
			m_OnDestroyed = new ScriptInvokerEntity();
		
		return m_OnDestroyed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create a new group and add the character to the group (Server only)
	//! \return the created group
	SCR_EditableEntityComponent CreateGroupForCharacter()
	{
		if (!m_Agent)
			return null;

		GenericEntity owner = GetOwner();
		if (!owner)
			return null;

		//--- Create group
		SCR_EditableCharacterComponentClass prefabData = SCR_EditableCharacterComponentClass.Cast(GetEditableEntityData());
		if (!prefabData)
			return null;

		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform[3] = m_Agent.GetControlledEntity().GetOrigin();
		IEntity groupEntity = GetGame().SpawnEntityPrefab(Resource.Load(prefabData.GetPrefabGroup()), owner.GetWorld(), params);
		if (!groupEntity)
			return null;

		AIGroup group = AIGroup.Cast(groupEntity);
		if (!group)
		{
			Print(string.Format("Cannot create group, prefab '%1' is not AIGroup entity!", prefabData.GetPrefabGroup()), LogLevel.ERROR);
			delete groupEntity;
			return null;
		}

		SCR_EditableEntityComponent groupEditableEntity = SCR_EditableEntityComponent.GetEditableEntity(groupEntity);
		if (!groupEditableEntity)
		{
			Print(string.Format("Cannot create group, prefab '%1' does not contain SCR_EditableEntityComponent!", prefabData.GetPrefabGroup()), LogLevel.ERROR);
			delete groupEntity;
			return null;
		}

		//--- Set group's faction
		SCR_AIGroup groupScripted = SCR_AIGroup.Cast(group);
		if (groupScripted)
		{
			FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
			if (factionComponent)
			{
				Faction faction = factionComponent.GetAffiliatedFaction();
				if (faction)
					groupScripted.InitFactionKey(faction.GetFactionKey());
			}
		}

		//--- Add the entity to the group
		group.AddAgent(m_Agent);
//
		//group.SetWorldTransform(matrix);

		return groupEditableEntity;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsPlayer(IEntity owner = null)
	{
		if (m_inDeadPlayerID != 0)
			return true;

		if (owner)
			return SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(owner) > 0;
		else if (m_Owner)
			return SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(m_Owner) > 0;
		else
			return m_Owner == SCR_PlayerController.GetLocalMainEntity();
	}

	//------------------------------------------------------------------------------------------------
	//~ Check if local player is owner of entity
	protected bool IsLocalPlayerOwner()
	{
		//~ Not replicated so always owner
		if (!IsReplicated())
			return true;

		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		return !rplComp || rplComp.IsOwner();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if character is Player or Possessed by a player
	//! \return true if controlled by player
	bool IsPlayerOrPossessed()
	{
		return !m_AgentControlComponent || !m_AgentControlComponent.IsAIActivated();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState != ECharacterLifeState.DEAD)
			return;
		
		//--- Cache player's ID upon death, so we can access it later
		m_inDeadPlayerID = SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(GetOwner());

		m_OnUIRefresh.Invoke();
		
		if (m_OnDestroyed)
			m_OnDestroyed.Invoke(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override bool Delete(bool changedByUser = false, bool updateNavmesh = true)
	{
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(GetOwner().FindComponent(SCR_CompartmentAccessComponent));
		if (compartmentAccess)
		{
			BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
			if (slot)
				slot.SetCompartmentAccessible(true);
		}

		return super.Delete(changedByUser, updateNavmesh);
	}

	//------------------------------------------------------------------------------------------------
	//! Called by SCR_EditableGroupComponent when faction is changed
	void OnFactionChanged()
	{
		m_OnUIRefresh.Invoke();
	}

	//------------------------------------------------------------------------------------------------
//	override bool CanSetParent(SCR_EditableEntityComponent parentEntity)
//	{
//		if (!parentEntity)
//			return true;
//
//		EEditableEntityType type = parentEntity.GetEntityType();
//		return type == EEditableEntityType.GROUP || type == EEditableEntityType.CHARACTER || type == EEditableEntityType.VEHICLE;
//	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected override void SetTransformOwner(vector transform[4])
	{
		super.SetTransformOwner(transform);

		PlayerTeleportedFeedback(false);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void PlayerTeleportedFeedback(bool isLongFade)
	{
		if (!m_Owner)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;

		if (m_Owner != playerController.GetLocalControlledEntity())
			return;

		SCR_PlayerTeleportedFeedbackComponent playerTeleportedComponent = SCR_PlayerTeleportedFeedbackComponent.Cast(playerController.FindComponent(SCR_PlayerTeleportedFeedbackComponent));
		if (!playerTeleportedComponent)
			return;

		playerTeleportedComponent.PlayerTeleported(GetOwner(), isLongFade, SCR_EPlayerTeleportedReason.EDITOR);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when character was moved by Parent (aka when in vehicle)
	//! Checks if player controlled and if Owned by local player. If true shows feedback
	//!
	void PlayerTeleportedByParentFeedback(bool isLongFade)
	{
		if (IsPlayerOrPossessed())
			Rpc(PlayerTeleportedFeedback, isLongFade);
	}

	//------------------------------------------------------------------------------------------------
	//--- #HACK - when the game is paused, unpause it to ensure that the character is moved in/out of a compartment, which is an operation that needs simulation to be running
	protected void UpdateCompartment(IEntity newParent)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (world.IsGameTimePaused())
		{
			world.PauseGameTime(false);
			GetGame().GetCallqueue().CallLater(WaitForCompartmentUpdate, 1, true, newParent);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void WaitForCompartmentUpdate(IEntity newParent)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (world.IsGameTimePaused() || !GetOwner() || CompartmentAccessComponent.GetVehicleIn(GetOwner()) == newParent)
		{
			world.PauseGameTime(true);
			GetGame().GetCallqueue().Remove(WaitForCompartmentUpdate);
		}
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent SetParentEntity(SCR_EditableEntityComponent parentEntity, bool changedByUser = false)
	{
		//--- When moving to root or non-AI-compatible layer, create a group for the character in that layer
		if (changedByUser && IsServer() && !GetOwner().IsDeleted() && !IsDestroyed())
		{
			AIControlComponent aiControl = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
			if (aiControl && aiControl.IsAIActivated())
			{
				EEditableEntityType parentType = EEditableEntityType.GENERIC;
				if (parentEntity)
					parentType = parentEntity.GetEntityType();

				if (parentType == EEditableEntityType.GENERIC)
				{
					SCR_EditableEntityComponent parentEntityPrev = parentEntity;
					parentEntity = CreateGroupForCharacter();
					parentEntity.SetParentEntity(parentEntityPrev);
					return parentEntity;
				}
			}
		}

		return super.SetParentEntity(parentEntity);
	}

	//------------------------------------------------------------------------------------------------
	//~ Authority Only. When spawned will force characters into vehicle position
	override void ForceVehicleCompartments(notnull array<ECompartmentType> forceVehicleCompartments)
	{
		if (forceVehicleCompartments.IsEmpty())
			return;

		m_aForceVehicleCompartments = {};

		foreach (ECompartmentType compartment : forceVehicleCompartments)
		{
			m_aForceVehicleCompartments.Insert(compartment);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnParentEntityChanged(SCR_EditableEntityComponent parentEntity, SCR_EditableEntityComponent parentEntityPrev, bool changedByUser)
	{
		EEditableEntityType parentType;
		if (parentEntity)
			parentType = parentEntity.GetEntityType();

		switch (parentType)
		{
			case EEditableEntityType.VEHICLE:
			{
				//~ Checks if called for owner and is Player or possessed. Show teleport feedback
				if (IsPlayerOrPossessed() && IsLocalPlayerOwner())
					PlayerTeleportedFeedback(true);

				//--- Execute on server, MoveInVehicle handles distribution to clients
				if (Replication.IsServer())
				{
					Faction characterFaction = GetFaction();
					Faction vehicleFaction = parentEntity.GetFaction();
					//Is vehicle hostile? -> if so, do nothing
					if (vehicleFaction && characterFaction.IsFactionEnemy(vehicleFaction))
					{
						Event_OnCharacterMovedInVehicle.Invoke(this, null);
						break;
					};

					SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(GetOwner().FindComponent(SCR_CompartmentAccessComponent));
					if (compartmentAccess)
					{
						GenericEntity parentOwner = parentEntity.GetOwner();
						bool isAi = !IsPlayerOrPossessed();

						//Add vehicle to group
						if (isAi)
							AddUsableVehicle(parentOwner);

						ChimeraWorld world = GetGame().GetWorld();
						//~ Try placing character into vehicle
						if (!m_aForceVehicleCompartments || m_aForceVehicleCompartments.IsEmpty())
						{
							//~ Try to add character to any free vehicle slot
							if (compartmentAccess.MoveInVehicle(parentOwner, ECompartmentType.PILOT, world.IsGameTimePaused()))
							{
								UpdateCompartment(parentOwner);
								Event_OnCharacterMovedInVehicle.Invoke(this, parentOwner);
								break;
							}
							if (compartmentAccess.MoveInVehicle(parentOwner, ECompartmentType.TURRET, world.IsGameTimePaused()))
							{
								UpdateCompartment(parentOwner);
								Event_OnCharacterMovedInVehicle.Invoke(this, parentOwner);
								break;
							}
							if (compartmentAccess.MoveInVehicle(parentOwner, ECompartmentType.CARGO, world.IsGameTimePaused()))
							{
								UpdateCompartment(parentOwner);
								Event_OnCharacterMovedInVehicle.Invoke(this, parentOwner);
								break;
							}
						}

						//~ Force character into vehicle. Will delete the character if it fails to find a valid empty compartment
						else
						{
							if (m_aForceVehicleCompartments.Contains(ECompartmentType.PILOT) && compartmentAccess.MoveInVehicle(parentOwner, ECompartmentType.PILOT, world.IsGameTimePaused()))
							{
								UpdateCompartment(parentOwner);
								Event_OnCharacterMovedInVehicle.Invoke(this, parentOwner);
								break;
							}

							if (m_aForceVehicleCompartments.Contains(ECompartmentType.TURRET) && compartmentAccess.MoveInVehicle(parentOwner, ECompartmentType.TURRET, world.IsGameTimePaused()))
							{
								UpdateCompartment(parentOwner);
								Event_OnCharacterMovedInVehicle.Invoke(this, parentOwner);
								break;
							}

							if (m_aForceVehicleCompartments.Contains(ECompartmentType.CARGO) && compartmentAccess.MoveInVehicle(parentOwner, ECompartmentType.CARGO, world.IsGameTimePaused()))
							{
								UpdateCompartment(parentOwner);
								Event_OnCharacterMovedInVehicle.Invoke(this, parentOwner);
								break;
							}

							//~ Failed to move in vehicle means delete character
							Event_OnCharacterMovedInVehicle.Invoke(this, null);

							//~ Delete the next frame so entity is correctly initialized and delete is valid
							GetGame().GetCallqueue().CallLater(Delete, param1: false, param2: true);
						}

						//~ Failed to move in vehicle
						Event_OnCharacterMovedInVehicle.Invoke(this, null);
					}
				}

				break;
			}
			case EEditableEntityType.GROUP:
			case EEditableEntityType.CHARACTER:
			{
				if (!parentEntity)
					break;

				SCR_EditableEntityComponent group = parentEntity.GetAIGroup();
				super.OnParentEntityChanged(group, parentEntityPrev, changedByUser);
				m_OnUIRefresh.Invoke(); //--- Update GUI when character moves to another group (in case it's a group of a different faction)
				break;
			}
			default:
			{
				//--- Register player
				super.OnParentEntityChanged(null, parentEntityPrev, changedByUser);
			}
		}

		if (m_aForceVehicleCompartments)
			m_aForceVehicleCompartments = null;

		//--- Create a new group (ToDo: Solve together with entity<->entity interaction)
		//SCR_EditableEntityComponent newGroup = CreateGroupForCharacter();
		//SetParentEntity(newGroup);
	}

	//------------------------------------------------------------------------------------------------
	override void SetTransform(vector transform[4], bool changedByUser = false)
	{
		//--- Make sure characters don't fall through ground
		transform[3][1] = Math.Max(transform[3][1], GetOwner().GetWorld().GetSurfaceY(transform[3][0], transform[3][2]));
		
		//--- Move out of a vehicle
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(GetOwner().FindComponent(CompartmentAccessComponent));
		if (compartmentAccess && compartmentAccess.IsInCompartment())
		{
			//~Todo: The delay is not ideal. It would need to be checked if all characters are dragged out of the vehicle in some other way
			//Unregister from vehicle on next frame (checks with delay as all characters might be dragged out at the same time and it would think the vehicle is still occupied)
			if (IsServer())
				GetGame().GetCallqueue().CallLater(RemoveUsableVehicle, 100, false, GetVehicle().GetOwner(), true);

			//~ If moved out vehicle by GM. Check if player or possessed. If true send teleport feedback request
			if (changedByUser && IsPlayerOrPossessed())
				Rpc(PlayerTeleportedFeedback, false);

			// Get out of vehicle 
			RplComponent slotRplComponent = RplComponent.Cast(compartmentAccess.GetOwner().FindComponent(RplComponent));
			RplId slotEntityID = slotRplComponent.Id();
			
			Rpc(GetOutVehicleOwner, slotEntityID, transform);

			return;
		}

		super.SetTransform(transform, changedByUser);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void GetOutVehicleOwner(RplId slotEntityID, vector transform[4])
	{
		RplComponent slotRplComponent = RplComponent.Cast(Replication.FindItem(slotEntityID));
		IEntity slotEntity = slotRplComponent.GetEntity();
		
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(slotEntity.FindComponent(CompartmentAccessComponent));
		
		compartmentAccess.GetOutVehicle_NoDoor(transform, false, true);
		UpdateCompartment(null);
	}

	//------------------------------------------------------------------------------------------------
	override int GetPlayerID()
	{
		if (IsDestroyed())
			return m_inDeadPlayerID;
		else
			return SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override Faction GetFaction()
	{
		//--- Destroyed entities have no faction
		if (IsDestroyed())
			return null;

		//--- Get player faction
		int playerID = GetPlayerID();
		if (playerID > 0)
		{
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (factionManager)
			{
				Faction faction = factionManager.GetPlayerFaction(playerID);
				if (faction)
					return faction;
			}
		}

		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(m_Owner.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
			return factionAffiliation.GetAffiliatedFaction();
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnUIRefresh()
	{
		return m_OnUIRefresh;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent GetAIGroup()
	{
		return GetParentEntity();
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent GetAIEntity()
	{
		return this;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent GetVehicle()
	{
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(GetOwner().FindComponent(SCR_CompartmentAccessComponent));
		if (compartmentAccess)
			return SCR_EditableEntityComponent.GetEditableEntity(compartmentAccess.GetVehicle());
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override int GetCrew(out notnull array<CompartmentAccessComponent> crewCompartmentAccess, bool ignorePlayers = true)
	{
		//Ignore player
		if (ignorePlayers)
		{
			if (GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(GetOwner()) > 0)
				return 0;
		}

		//Check if is in vehicle
		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(GetOwner().FindComponent(CompartmentAccessComponent));
		if (compartmentAccess && compartmentAccess.IsInCompartment())
			crewCompartmentAccess.Insert(compartmentAccess);

		return crewCompartmentAccess.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Add useable vehicle to group which registers the vehicle to the group. Call on server.
	//! \param vehicle
	void AddUsableVehicle(IEntity vehicle)
	{
		if (IsPlayer() || !GetAIGroup())
			return;

		SCR_EditableGroupComponent group = SCR_EditableGroupComponent.Cast(GetAIGroup());
		if (!group)
			return;

		SCR_AIGroup aiGroup = group.GetAIGroupComponent();
		if (!aiGroup)
			return;
		
		SCR_AIVehicleUsageComponent vehicleUsageComp = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicle, vehicle);
		if (!vehicleUsageComp)
			return;
				
		aiGroup.GetGroupUtilityComponent().m_VehicleMgr.TryAddVehicle(vehicleUsageComp);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove useable vehicle from group which unregisters the vehicle from the group. Call on server.
	//! NOTE: If All characters of a group are told to exit the vehicle at the same time and checkIfVehicleStillUsed is true, then the function will think the vehicle is still in use by the group!
	//! \param vehicle IEntity vehicle
	//! \param checkIfVehicleStillUsed
	void RemoveUsableVehicle(IEntity vehicle, bool checkIfVehicleStillUsed = true)
	{
		if (IsPlayer() || !GetAIGroup())
			return;

		SCR_EditableGroupComponent group = SCR_EditableGroupComponent.Cast(GetAIGroup());
		if (!group)
			return;

		SCR_AIGroup aiGroup = group.GetAIGroupComponent();
		if (!aiGroup)
			return;
		
		
		SCR_AIVehicleUsageComponent vehicleUsageComp = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicle, vehicle);
		if (!vehicleUsageComp)
			return;
		
		SCR_AIGroupVehicleManager vehMgr = aiGroup.GetGroupUtilityComponent().m_VehicleMgr;
		//Check if vehicle is used by group
		if (vehMgr.FindVehicle(vehicleUsageComp) == null)
			return;

		if (checkIfVehicleStillUsed)
		{
			set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
			group.GetChildren(entities, true);

			SCR_EditableVehicleComponent editableVehicle = SCR_EditableVehicleComponent.Cast(vehicle.FindComponent(SCR_EditableVehicleComponent));
			if (editableVehicle)
			{
				SCR_EditableEntityComponent crew;
				array<CompartmentAccessComponent> crewCompartmentAccess = {};
				editableVehicle.GetCrew(crewCompartmentAccess, true);

				foreach (CompartmentAccessComponent compartment : crewCompartmentAccess)
				{
					crew = SCR_EditableEntityComponent.Cast(compartment.GetOwner().FindComponent(SCR_EditableEntityComponent));

					if (crew == null || crew == this)
						continue;

					//Vehicle is still used by group
					if (entities.Contains(crew) && !compartment.IsGettingOut())
						return;
				}
			}
		}

		//Remove vehicle
		vehMgr.RemoveVehicle(vehicleUsageComp);
	}

//	//------------------------------------------------------------------------------------------------
//	//! Remove all useable vehicle from group which unregisters all vehicles from the group. Call on server.
//	void RemoveAllUsableVehicles()
//	{
//		if (IsPlayer() || !GetAIGroup())
//			return;
//
//		array<IEntity> usableVehicles = new array<IEntity>;
//		m_Group.GetUsableVehicles(usableVehicles);
//
//		foreach (IEntity vehicle : usableVehicles)
//			m_Group.RemoveUsableVehicle(vehicle);
//	}

	//------------------------------------------------------------------------------------------------
	override float GetMaxDrawDistanceSq()
	{
		if (HasEntityState(EEditableEntityState.PLAYER))
		{
			if (m_bShouldRecalculateDrawDistance || m_fPlayerDrawDistance == 0)
			{
				SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));

				SCR_EditableEntityComponent vehicle = GetVehicle();
				int playerId = GetPlayerID();

				bool isPlayerInVehicle = vehicle && playerId > 0;
				m_fPlayerDrawDistance = core.GetPlayerDrawDistanceSq(isPlayerInVehicle);

				m_bShouldRecalculateDrawDistance = false;
			}

			return m_fPlayerDrawDistance;
		}
		else
		{
			return m_fMaxDrawDistance;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] compartmentEntity
	void OnPlayerCharacterEnterCompartment(IEntity compartmentEntity)
	{
		m_bShouldRecalculateDrawDistance = true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] compartmentEntity
	void OnPlayerCharacterExitCompartment(IEntity compartmentEntity)
	{
		m_bShouldRecalculateDrawDistance = true;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent EOnEditorPlace(out SCR_EditableEntityComponent parent, SCR_EditableEntityComponent recipient, EEditorPlacingFlags flags, bool isQueue, int playerID = 0)
	{	
		GenericEntity owner = GetOwner();

		//--- Entity is a player, don't activate AI
		if (flags & EEditorPlacingFlags.CHARACTER_PLAYER)
			return this;

		//--- Activate AI
		m_AgentControlComponent = AIControlComponent.Cast(owner.FindComponent(AIControlComponent));
		if (m_AgentControlComponent)
			m_AgentControlComponent.ActivateAI();
		
		if (parent && AIGroup.Cast(parent.GetOwner()))
		{
			//--- Creating inside of a group - add character to it
			return this;
		}
		else
		{
			//--- Creating outside of a group - create one for the character
			SCR_EditableEntityComponent group = CreateGroupForCharacter();
			if (group)
				return group;
			else
				return this;
		}
 	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnEditorSessionLoad(SCR_EditableEntityComponent parent)
	{
		//--- Activate AI
		AIControlComponent control = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
		if (control)
			control.ActivateAI();
	}

	/*
	//------------------------------------------------------------------------------------------------
	override bool Serialize(out SCR_EditableEntityComponent outTarget = null, out int outTargetIndex = -1, out EEditableEntitySaveFlag outSaveFlags = 0)
	{
		//--- Save SP player
		if (!Replication.IsRunning() && IsPlayer())
			outSaveFlags |= EEditableEntitySaveFlag.PLAYER;
			
		if (super.Serialize(outTarget, outTargetIndex, outSaveFlags))
		{
			SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(GetOwner().FindComponent(SCR_CompartmentAccessComponent));
			if (compartmentAccess)
				outTarget = SCR_EditableEntityComponent.GetEditableEntity(compartmentAccess.GetVehicle(outTargetIndex));

			return true;
		}
		else
		{
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Deserialize(SCR_EditableEntityComponent target, int targetValue)
	{
		if (!target)
			return;

		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(target.GetOwner().FindComponent(BaseCompartmentManagerComponent));
		array<BaseCompartmentSlot> compartments = {};
		compartmentManager.GetCompartments(compartments);

		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(GetOwner().FindComponent(SCR_CompartmentAccessComponent));
		compartmentAccess.GetInVehicle(target.GetOwner(), compartments[targetValue], true, -1, ECloseDoorAfterActions.INVALID, true);
	}
	*/

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_DISABLE))
			return;

		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode(owner))
			return;

		if (Replication.IsServer())
		{
			if (GetEntityType() != EEditableEntityType.CHARACTER)
			{
				Print(string.Format("SCR_EditableCharacterComponent must have type set to CHARACTER, it's %1!", typename.EnumToString(EEditableEntityType, GetEntityType())), LogLevel.ERROR);
				return;
			}

			m_AgentControlComponent = AIControlComponent.Cast(m_Owner.FindComponent(AIControlComponent));
			if (m_AgentControlComponent)
				m_Agent = m_AgentControlComponent.GetControlAIAgent();
		}
		
		ChimeraCharacter char = ChimeraCharacter.Cast(owner);
		if (char)
		{
			SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(char.FindComponent(SCR_CharacterControllerComponent));
			if (charController)
				charController.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
			else
				Print(string.Format("Failed to insert listened for lifestate changing on SCR_EditableEntityComponent of: " + GetOwner()), LogLevel.ERROR);
		}
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_EditableCharacterComponent()
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (char)
		{
			SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
			if (charController)
				charController.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
		}
	}	
}
