void OnPlayerEnterCompartmentMethod(ChimeraCharacter playerCharacter, IEntity compartmentEntity);
typedef func OnPlayerEnterCompartmentMethod;
typedef ScriptInvokerBase<OnPlayerEnterCompartmentMethod> OnPlayerEnterCompartment;

void OnPlayerExitCompartmentMethod(ChimeraCharacter playerCharacter, IEntity compartmentEntity);
typedef func OnPlayerExitCompartmentMethod;
typedef ScriptInvokerBase<OnPlayerEnterCompartmentMethod> OnPlayerExitCompartment;

[EntityEditorProps(category: "GameScripted/Vehicle", description: "CompartmentAccessComponent", color: "0 0 255 255")]
class SCR_CompartmentAccessComponentClass: CompartmentAccessComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CompartmentAccessComponent : CompartmentAccessComponent
{	
	//! Local invokers for a specific vehicle
	private ref ScriptInvoker m_OnCompartmentEntered;
	private ref ScriptInvoker m_OnCompartmentLeft;
	
	protected ref OnPlayerEnterCompartment m_OnPlayerEnterCompartment;
	protected ref OnPlayerExitCompartment m_OnPlayerExitCompartment;

	protected SCR_FireModeManagerComponent m_TurretFireModeManager;
	
	protected const int WAIT_FOR_VEHICLE_MAX_TRIES = 10;
	protected static int s_iWaitForVehicleTries = 0;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCompartmentEntered(bool createNew = true)
	{
		if (!m_OnCompartmentEntered && createNew)
			m_OnCompartmentEntered = new ScriptInvoker();
		return m_OnCompartmentEntered;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCompartmentLeft(bool createNew = true)
	{
		if (!m_OnCompartmentLeft && createNew)
			m_OnCompartmentLeft = new ScriptInvoker();
		return m_OnCompartmentLeft;
	}

	//------------------------------------------------------------------------------------------------
	OnPlayerEnterCompartment GetOnPlayerCompartmentEnter(bool createNew = true)
	{
		if (!m_OnPlayerEnterCompartment && createNew)
			m_OnPlayerEnterCompartment = new OnPlayerEnterCompartment();

		return m_OnPlayerEnterCompartment;
	}

	//------------------------------------------------------------------------------------------------
	OnPlayerExitCompartment GetOnPlayerCompartmentExit(bool createNew = true)
	{
		if (!m_OnPlayerExitCompartment && createNew)
			m_OnPlayerExitCompartment = new OnPlayerExitCompartment();

		return m_OnPlayerExitCompartment;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (m_OnCompartmentEntered)
			m_OnCompartmentEntered.Invoke( targetEntity, manager, mgrID, slotID, move );
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());

		if (!playerManager || !character)
			return;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (controller)
			controller.m_OnLifeStateChanged.Insert(OnLifeStateChanged);

		BaseCompartmentSlot slot = GetCompartment();
		if (slot)
			ToggleTurretFireModeControlls(slot, true);
		
		//--- Check if the character is a player
		int playerId = playerManager.GetPlayerIdFromControlledEntity(character);
		if (playerId == 0)
			return;

		if (m_OnPlayerEnterCompartment)
			m_OnPlayerEnterCompartment.Invoke(character, targetEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Updates the inputs for fire mode manager of a turret
	//! \param[in] slot for which this should be exectued
	//! \param[in] entered
	protected void ToggleTurretFireModeControlls(notnull BaseCompartmentSlot slot, bool entered)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		IEntity localCharacter = SCR_PlayerController.GetLocalControlledEntity();
		if (!entered && character == localCharacter)
		{
			if (m_TurretFireModeManager)
				m_TurretFireModeManager.RemoveActionListeners();

			m_TurretFireModeManager = null;
			return;
		}

		TurretControllerComponent turretController = slot.GetAttachedTurret();
		Turret turret;
		if (turretController)
		{
			turret = Turret.Cast(turretController.GetOwner());
		}
		else
		{
			string turretName = slot.GetAutoConnectTurret();
			if (turretName.IsEmpty())
				return;

			IEntity child = slot.GetOwner().GetChildren();
			while (child)
			{
				turret = Turret.Cast(child);
				if (turret)
				{
					turretController = TurretControllerComponent.Cast(turret.FindComponent(TurretControllerComponent));
					if (turretController && turretController.GetUniqueName() == turretName)
						break;
				}

				child = child.GetSibling();
			}

			if (!turret)
				return;
		}

		m_TurretFireModeManager = SCR_FireModeManagerComponent.Cast(turret.FindComponent(SCR_FireModeManagerComponent));
		if (!m_TurretFireModeManager)
			return;

		if (character == localCharacter)
			m_TurretFireModeManager.SetUpAllActionListeners(character);
	}

	//------------------------------------------------------------------------------------------------
	SCR_FireModeManagerComponent GetControlledFireModeManager()
	{
		return m_TurretFireModeManager;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (m_OnCompartmentLeft)
			m_OnCompartmentLeft.Invoke( targetEntity, manager, mgrID, slotID, move );

		PlayerManager playerManager = GetGame().GetPlayerManager();
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());

		if (!playerManager || !character)
			return;

		BaseCompartmentSlot slot = manager.FindCompartment(slotID);
		if (slot)
			ToggleTurretFireModeControlls(slot, false);

		//--- Check if the character is a player
		int playerId = playerManager.GetPlayerIdFromControlledEntity(character);
		if (playerId == 0)
			return;

		if (m_OnPlayerExitCompartment)
			m_OnPlayerExitCompartment.Invoke(character, targetEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		IEntity vehicle = GetVehicle();
		if (!vehicle)
			return;
		
		SCR_VehicleFactionAffiliationComponent vehicleFactionAff = SCR_VehicleFactionAffiliationComponent.Cast(vehicle.FindComponent(SCR_VehicleFactionAffiliationComponent));
		if (!vehicleFactionAff)
			return;
		
		vehicleFactionAff.OnOccupantLifeStateChanged(previousLifeState, newLifeState);	
	}
	
	/*!
	Find a compartment that is marked as accessible and not occupied by anyone.
	\param vehicle Target vehicle
	\param compartmentType Type of compartment
	\return Compartment, null if none was found
	*/
	BaseCompartmentSlot FindFreeAndAccessibleCompartment(IEntity vehicle, ECompartmentType compartmentType, BaseCompartmentSlot customSlot = null)
	{
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return null;
		
		array<BaseCompartmentSlot> compartments = {};
		compartmentManager.GetCompartments(compartments);
		
		if (customSlot)
		{
			if (!customSlot.IsCompartmentAccessible())
				return null;
			if (customSlot.IsGetInLockedFor(GetOwner()))
				return null;
			if (customSlot.GetOccupant())
				return null;
			if (compartmentType != customSlot.GetType())
				return null;
			
			return customSlot;
		}

		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (!compartment.IsCompartmentAccessible())
				continue;
			if (compartment.IsGetInLockedFor(GetOwner()))
				continue;
			if (compartment.GetOccupant())
				continue;
			if (compartmentType != compartment.GetType())
				continue;
			
			return compartment;
		}
		
		//--- Scan compartments in child entities (ToDo: Remove once GetCompartments is recursive)
		BaseCompartmentSlot childSlot;
		IEntity child = vehicle.GetChildren();
		while (child)
		{
			childSlot = FindFreeAndAccessibleCompartment(child, compartmentType);
			if (childSlot)
				return childSlot;
			
			child = child.GetSibling();
		}
		
		return null;
	}
	
	/*!
	Move in vehicle to specified slot type.
	When executed on server while the character is owned by a client, this function will send the request there.
	\param vehicle Target vehicle
	\param compartmentType Type of compartment
	\return True if the request was successful
	*/
	bool MoveInVehicle(IEntity vehicle, ECompartmentType compartmentType, bool performWhenPaused = false, BaseCompartmentSlot customSlot = null)
	{
		if (!vehicle)
			return false;
		
		//--- Find suitable slot
		BaseCompartmentSlot slot = FindFreeAndAccessibleCompartment(vehicle, compartmentType, customSlot);
		if (!slot)
			return false;

		//--- Move character in the slot (must be called where character is local)
		IEntity slotEntity = slot.GetOwner();
		RplComponent slotRplComponent = RplComponent.Cast(slotEntity.FindComponent(RplComponent));
		RplId slotEntityID = slotRplComponent.Id();
		int slotID = slot.GetCompartmentSlotID();
		Rpc(MoveInVehicleOwner, slotEntityID, slotID, performWhenPaused);
		
		//--- Lock the slot for a frame, so any other MoveInVehicle calls in this frame will skip it
		slot.SetCompartmentAccessible(false);
		GetGame().GetCallqueue().CallLater(slot.SetCompartmentAccessible, 1, false, true);
		return true;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void MoveInVehicleOwner(RplId slotEntityID, int slotID, bool performWhenPaused)
	{
		RplComponent slotRplComponent = RplComponent.Cast(Replication.FindItem(slotEntityID));
		if (!slotRplComponent)
		{
			if (s_iWaitForVehicleTries >= WAIT_FOR_VEHICLE_MAX_TRIES)
			{
				// Time out, abort
				s_iWaitForVehicleTries = 0;
				return;
			}
			
			GetGame().GetCallqueue().CallLater(MoveInVehicleOwner, 1000, false, slotEntityID, slotID, performWhenPaused);
			s_iWaitForVehicleTries++;
			return;
		}
		
		s_iWaitForVehicleTries = 0;
		
		IEntity slotEntity = slotRplComponent.GetEntity();
		
		BaseCompartmentManagerComponent slotCompartmentManager = BaseCompartmentManagerComponent.Cast(slotEntity.FindComponent(BaseCompartmentManagerComponent));
		BaseCompartmentSlot slot = slotCompartmentManager.FindCompartment(slotID);
		
		//--- Use slot's owner, not the vehicle - the slot may be in vehicle's child entity
		GetInVehicle(slotEntity, slot, true, -1, ECloseDoorAfterActions.INVALID, performWhenPaused);
	}
	
	/*!
	Move in vehicle to any available slot.
	When executed on server while the character is owned by a client, this function will send the request there.
	\param vehicle Target vehicle
	\return True if the request was successful
	*/
	bool MoveInVehicleAny(IEntity vehicle)
	{
		if (!vehicle)
			return false;
		
		ChimeraWorld world = GetGame().GetWorld();
		if (MoveInVehicle(vehicle, ECompartmentType.PILOT, world.IsGameTimePaused()))
			return true;
		
		if (MoveInVehicle(vehicle, ECompartmentType.TURRET, world.IsGameTimePaused()))
			return true;
		
		if (MoveInVehicle(vehicle, ECompartmentType.CARGO, world.IsGameTimePaused()))
			return true;
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to ask the owner to leave the compartment
	//! \param[in] type how player should get out of the vehicle
	//! \param[in] doorInfoIndex through which door player will try to get out
	//! \param[in] closeDoor should player close the door when he leaves the vehicle
	//! \param[in] performWhenPaused
	//! \param[in] ejectOnTheSpot if character should be ejected on the position where he currently is
	void AskOwnerToGetOutFromVehicle(EGetOutType type, int doorInfoIndex, ECloseDoorAfterActions closeDoor, bool performWhenPaused, bool ejectOnTheSpot = false)
	{
		if (!GetCompartment())
			return;
 
		Rpc(GetOutOwner, type, doorInfoIndex, closeDoor, performWhenPaused, ejectOnTheSpot);
	}

	//------------------------------------------------------------------------------------------------
	//! Replicated method executed by the owner of the character in order to get out of the vehicle
	//! \param[in] type how player should get out of the vehicle
	//! \param[in] doorInfoIndex through which door player will try to get out
	//! \param[in] closeDoor should player close the door when he leaves the vehicle
	//! \param[in] performWhenPaused
	//! \param[in] ejectOnTheSpot if character should be ejected on the position where he currently is
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void GetOutOwner(EGetOutType type, int doorInfoIndex, ECloseDoorAfterActions closeDoor, bool performWhenPaused, bool ejectOnTheSpot)
	{
		if (ejectOnTheSpot)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
			if (!character)
				return;

			vector mat[4];
			character.GetTransform(mat);
			mat[3] = character.AimingPosition();

			GetOutVehicle_NoDoor(mat, true, false);
			return;
		}

		GetOutVehicle(type, doorInfoIndex, closeDoor, performWhenPaused);
	}
	
	/*!
	Get vehicle this entity is in.
	Works even when the entity is not directly in the vehicle, but in one of its child entities (e.g., turrets)
	Use this function when you specifically need pointer to the vehicle. If you want to simply check if a character is in a vehicle, use IsInCompartment()!
	\return Vehicle, null when not in any compartment
	*/
	IEntity GetVehicle()
	{
		if (GetCompartment())
			return GetCompartment().GetVehicle();
		else
			return null;
	}
	IEntity GetVehicle(out int compartmentID)
	{
		if (GetCompartment())
			return GetCompartment().GetVehicle(compartmentID);
		else
			return null;
	}
	
	void ~SCR_CompartmentAccessComponent()
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
		if (!controller)
			return;
		
		controller.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
	}
	
	/*!
	Get type of a compartment.
	\param compartment Queried compartment
	\return Compartment type, or -1 if invalid
	*/
	[Obsolete("Use GetType() on the compartment instance directly.")]
	static ECompartmentType GetCompartmentType(notnull BaseCompartmentSlot compartment)
	{
		return compartment.GetType();
	}
};
