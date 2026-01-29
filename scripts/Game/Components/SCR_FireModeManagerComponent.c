//------------------------------------------------------------------------------------------------
void OnTurretFireModeValuesChanged(EWeaponGroupFireMode fireMode, int quantity, int weaponGroupId);
typedef func OnTurretFireModeValuesChanged;
typedef ScriptInvokerBase<OnTurretFireModeValuesChanged> OnTurretFireModeValuesChangedInvoker;

class SCR_FireModeManagerComponentClass: ScriptComponentClass
{
};

class SCR_FireModeManagerComponent : ScriptComponent
{
	[Attribute(desc: "Add weapons groups to fire multiple weapons per shot. See SCR_WeaponsGroup")]
	protected ref array<ref SCR_WeaponGroup> m_aWeaponGroups;
	
	protected TurretControllerComponent turretController;
	protected EWeaponGroupFireMode m_eSetFireMode;
	protected ChimeraCharacter m_ControllingCharacter;
	protected ref array<int> m_aSetWeaponsGroup = {};
	protected ref OnTurretFireModeValuesChangedInvoker m_OnTurretFireModeValuesChanged;

	[RplProp(onRplName: "OnWeaponGroupBumped")]
	protected int m_iCurrentWeaponGroup;

	[RplProp(onRplName: "OnFireModeBumped")]
	protected EWeaponGroupFireMode m_eCurrentFireMode;

	[RplProp(onRplName: "OnRippleIntervalBumped")]
	protected float m_fRippleInterval = 100;

	[RplProp(onRplName: "OnRippleQuantityBumped")]
	protected int m_iRippleQuantity;

	protected const	string EVENT_NAME_ENTER_COMPARTMENT = "OnCompartmentEntered";

	protected const string ACTION_NAME_RIPPLE_QUANTITY = "TurretWeaponNextRippleQuantity";
	protected const string ACTION_NAME_FIRE_MODE = "TurretWeaponNextFireMode";
	protected const string ACTION_NAME_WEAPON_GROUP = "TurretNextWeapon";
	
	//------------------------------------------------------------------------------------------------
	protected void OnWeaponGroupBumped()
	{
		SetWeaponsGroup(m_iCurrentWeaponGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFireModeBumped()
	{
		SetFireMode(m_eCurrentFireMode);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRippleIntervalBumped()
	{
		SetRippleInterval(m_fRippleInterval);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRippleQuantityBumped()
	{
		SetRippleQuantity(m_iRippleQuantity);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	OnTurretFireModeValuesChangedInvoker GetOnTurretFireModeValuesChanged()
	{
		if (!m_OnTurretFireModeValuesChanged)
			m_OnTurretFireModeValuesChanged = new OnTurretFireModeValuesChangedInvoker();

		return m_OnTurretFireModeValuesChanged;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		turretController = TurretControllerComponent.Cast(owner.FindComponent(TurretControllerComponent));
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(owner.GetRootParent().FindComponent(EventHandlerManagerComponent));
		if (ev)
			ev.RegisterScriptHandler(EVENT_NAME_ENTER_COMPARTMENT, owner.GetRootParent(), OnCompartmentEntered, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! because weaponGroups are replicated over character controller, we set the base settings only once the first character enters
	//! \param[in] vehicle
	//! \param[in] mgr
	//! \param[in] occupant
	//! \param[in] managerId
	//! \param[in] slotID
	protected void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (m_aWeaponGroups.IsEmpty())
		{
			Print("SCR_FireModeManagerComponent not configured correctly on a turret on this vehicle: " + vehicle, LogLevel.WARNING);
			return;
		}

		if (!m_aWeaponGroups.IsIndexValid(m_iCurrentWeaponGroup))
			return;

		SetWeaponsGroup(m_iCurrentWeaponGroup);

		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(vehicle);
		if (rplComp && rplComp.Role() == RplRole.Authority)
			Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponGroupId
	protected void SetWeaponsGroup(int weaponGroupId)
	{
		SCR_WeaponGroup currentGroup = m_aWeaponGroups[weaponGroupId];
		if (!currentGroup)
			return;

		m_iCurrentWeaponGroup = weaponGroupId;
		SetWeaponsGroup(currentGroup.m_aWeaponsGroupIds);

		bool invoke = true;
		// if the new weapongroup does not support the current firemode, reset to the new groups default firemode
		array<int> availableFireModes = {};
		GetAvailableFireModes(availableFireModes);
		if (!availableFireModes.IsEmpty() && !availableFireModes.Contains(m_eCurrentFireMode))
		{
			SetFireMode(availableFireModes[0]);
			invoke = false;
		}

		if (invoke && m_OnTurretFireModeValuesChanged)
			m_OnTurretFireModeValuesChanged.Invoke(m_eCurrentFireMode, m_iRippleQuantity, m_iCurrentWeaponGroup);

		if (m_ControllingCharacter == SCR_PlayerController.GetLocalControlledEntity())
			SetUpFireModeActionListeners();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponsGroups
	protected void SetWeaponsGroup(array<int> weaponsGroups)
	{
		ConfigureWeaponsGroup(weapons: weaponsGroups);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] fireMode
	protected void SetFireMode(EWeaponGroupFireMode fireMode)
	{
		ConfigureWeaponsGroup(mode: fireMode);
		m_eCurrentFireMode = fireMode;

		bool invoke = true;
		if (m_eCurrentFireMode == EWeaponGroupFireMode.RIPPLE)
		{
			SCR_WeaponGroup currentGroup = m_aWeaponGroups[m_iCurrentWeaponGroup];
			if (currentGroup && !currentGroup.m_aRippleFireQuantities.IsEmpty() && !currentGroup.m_aRippleFireQuantities.Contains(m_iRippleQuantity))
			{
				SetRippleQuantity(currentGroup.m_aRippleFireQuantities[0]);
				invoke = false;
			}
		}

		if (invoke && m_OnTurretFireModeValuesChanged)
			m_OnTurretFireModeValuesChanged.Invoke(m_eCurrentFireMode, m_iRippleQuantity, m_iCurrentWeaponGroup);

		if (m_ControllingCharacter == SCR_PlayerController.GetLocalControlledEntity())
			SetUpRippleQuantityActionListeners();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] quantity
	protected void SetRippleQuantity(int quantity)
	{
		ConfigureWeaponsGroup(rippleQuantity: quantity);

		if (m_OnTurretFireModeValuesChanged)
			m_OnTurretFireModeValuesChanged.Invoke(m_eCurrentFireMode, m_iRippleQuantity, m_iCurrentWeaponGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] interval
	void SetRippleInterval(int interval)
	{
		ConfigureWeaponsGroup(rippleInterval: interval);
	}

	//------------------------------------------------------------------------------------------------
	protected void ConfigureWeaponsGroup(array<int> weapons = null, int mode = -1, int rippleQuantity = -1, int rippleInterval = -1)
	{
		if (!turretController)
			return;
		
		Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
		if (!vehicle)
			return;
		
		if (weapons)
		{
			turretController.SetWeaponGroup(weapons, m_eSetFireMode, m_iRippleQuantity, m_fRippleInterval);
			m_aSetWeaponsGroup = weapons;
		}
		else if (mode > -1)
		{
			turretController.SetWeaponGroup(m_aSetWeaponsGroup, mode, m_iRippleQuantity, m_fRippleInterval);
			m_eSetFireMode = mode;
		}
		else if (rippleQuantity > -1)
		{
			turretController.SetWeaponGroup(m_aSetWeaponsGroup, m_eSetFireMode, rippleQuantity, m_fRippleInterval);
			m_iRippleQuantity = rippleQuantity;
		}
		else if (rippleInterval > -1)
		{
			turretController.SetWeaponGroup(m_aSetWeaponsGroup, m_eSetFireMode, m_iRippleQuantity, rippleInterval);
			m_fRippleInterval = rippleInterval;		
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	void NextWeaponsGroup()
	{
		int desiredWeaponGroup = m_iCurrentWeaponGroup + 1;
		if (desiredWeaponGroup >= m_aWeaponGroups.Count())
			desiredWeaponGroup = 0;
		
		SetWeaponsGroup(desiredWeaponGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void NextFireMode()
	{
		array<int> availableFireModes = {};
		GetAvailableFireModes(availableFireModes);
		
		// new firemode is the next available firemode
		int currentFireModeId = availableFireModes.Find(GetFireMode());
		int desiredFireModeId = currentFireModeId + 1;
		if (desiredFireModeId >= availableFireModes.Count())
			desiredFireModeId = 0;
		
		SetFireMode(availableFireModes[desiredFireModeId]);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void NextRippleQuantity()
	{
		array<int> availableRippleQuantities = {};
		GetAvailableRippleQuantities(availableRippleQuantities);
		
		// new firemode is the next available firemode
		int currentRippleQuantityId = availableRippleQuantities.Find(GetRippleQuantity());
		int desiredRippleQuantityId = currentRippleQuantityId + 1;
		if (desiredRippleQuantityId >= availableRippleQuantities.Count())
			desiredRippleQuantityId = 0;
		
		SetRippleQuantity(availableRippleQuantities[desiredRippleQuantityId]);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNumberOfAvailableFireModes()
	{
		array<int> ints = {};
		return SCR_Enum.BitToIntArray(m_aWeaponGroups[m_iCurrentWeaponGroup].m_eFireMode, ints);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] availableFireModes
	void GetAvailableFireModes(notnull out array<int> availableFireModes)
	{
		availableFireModes.Clear();
		array<int> ints = {};		
		SCR_Enum.BitToIntArray(m_aWeaponGroups[m_iCurrentWeaponGroup].m_eFireMode, ints);

		foreach (int i : ints)
		{
			string fireMode = SCR_Enum.GetEnumName(SCR_EWeaponGroupFireMode, i);
			availableFireModes.Insert(typename.StringToEnum(EWeaponGroupFireMode, fireMode));
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void GetAvailableRippleQuantities(notnull out array<int> availableRippleQuantities)
	{
		availableRippleQuantities.Copy(m_aWeaponGroups[m_iCurrentWeaponGroup].m_aRippleFireQuantities);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] name
	//! \return
	int GetWeaponGroupID(out string name)
	{
		name = m_aWeaponGroups[m_iCurrentWeaponGroup].m_sWeaponGroupName;
		return m_iCurrentWeaponGroup;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] name
	//! \return
	SCR_WeaponGroup GetCurrentWeaponGroup(out string name)
	{
		name = m_aWeaponGroups[m_iCurrentWeaponGroup].m_sWeaponGroupName;
		return m_aWeaponGroups[m_iCurrentWeaponGroup];
	}
	
	//------------------------------------------------------------------------------------------------
	//!	\param[out] weaponGroups returns the m_aWeaponGroups array
	void GetAllWeaponGroups(out array<ref SCR_WeaponGroup> weaponGroups)
	{
		weaponGroups = m_aWeaponGroups;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] name
	//! \return
	EWeaponGroupFireMode GetFireMode(out string name = string.Empty)
	{
		name = SCR_Enum.GetEnumName(EWeaponGroupFireMode, m_eCurrentFireMode);
		return m_eCurrentFireMode;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] output
	//! \return
	int GetAvialableRippleQuantities(notnull array<int> output)
	{
		output = m_aWeaponGroups[m_iCurrentWeaponGroup].m_aRippleFireQuantities;
		return output.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRippleQuantity()
	{
		return m_iRippleQuantity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRippleInterval()
	{
		return m_fRippleInterval;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] change
	//! \param[in] newValue
	void ChangeFireModeValues(SCR_EFireModeChange change, int newValue)
	{
		switch (change)
		{
			case SCR_EFireModeChange.WEAPON_GROUP:
				if (!m_aWeaponGroups.IsIndexValid(newValue))
					return;

				SetWeaponsGroup(newValue);
				break;

			case SCR_EFireModeChange.FIRE_MODE:
				array<int> fireModes = {};
				GetAvailableFireModes(fireModes);
				if (fireModes.IsEmpty() || !fireModes.IsIndexValid(newValue))
					return;

				SetFireMode(fireModes[newValue]);
				break;

			case SCR_EFireModeChange.RIPPLE_QUANTITY:
				SCR_WeaponGroup currentGroup = m_aWeaponGroups[m_iCurrentWeaponGroup];
				if (currentGroup.m_aRippleFireQuantities.IsEmpty() || !currentGroup.m_aRippleFireQuantities.IsIndexValid(newValue))
					return;

				SetRippleQuantity(currentGroup.m_aRippleFireQuantities[newValue]);
				break;

			default:
				return;
		}

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method triggered by key press. Used to select ask the server to select next weapon group
	//! \param[in] value
	//! \param[in] reason
	//! \param[in] actionName
	protected void ReplicatedNextWeaponGroup(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (!m_ControllingCharacter)
		{
			RemoveActionListeners();
			return;
		}

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(m_ControllingCharacter.GetCharacterController());
		if (!controller)
			return;

		int numberOfWeaponGroups = m_aWeaponGroups.Count();
		if (numberOfWeaponGroups < 2)
		{
			SetUpWeaponGroupActionListeners();
			return;
		}

		int desiredWeaponGroup = m_iCurrentWeaponGroup + 1;
		if (desiredWeaponGroup >= numberOfWeaponGroups)
			desiredWeaponGroup = 0;

		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(GetOwner());
		controller.ReplicateTurretFireModeChange(SCR_EFireModeChange.WEAPON_GROUP, desiredWeaponGroup, rplComp.Id())
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method triggered by key press. Used to select ask the server to select next fire mode
	//! \param[in] value
	//! \param[in] reason
	//! \param[in] actionName
	protected void ReplicatedNextFireMode(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (!m_ControllingCharacter)
		{
			RemoveActionListeners();
			return;
		}

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(m_ControllingCharacter.GetCharacterController());
		if (!controller)
			return;

		array<int> availableFireModes = {};
		GetAvailableFireModes(availableFireModes);
		if (availableFireModes.IsEmpty() || availableFireModes.Count() < 2)
		{
			SetUpFireModeActionListeners();
			return;
		}
		
		int currentFireModeId = availableFireModes.Find(GetFireMode());
		int desiredFireModeId = currentFireModeId + 1;
		if (desiredFireModeId >= availableFireModes.Count())
			desiredFireModeId = 0;

		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(GetOwner());
		controller.ReplicateTurretFireModeChange(SCR_EFireModeChange.FIRE_MODE, desiredFireModeId, rplComp.Id())
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method triggered by key press. Used to select ask the server to select next ripple quantity
	//! \param[in] value
	//! \param[in] reason
	//! \param[in] actionName
	protected void ReplicatedNextRippleQuantity(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (!m_ControllingCharacter)
		{
			RemoveActionListeners();
			return;
		}

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(m_ControllingCharacter.GetCharacterController());
		if (!controller)
			return;

		array<int> availableRippleQuantities = {};
		GetAvailableRippleQuantities(availableRippleQuantities);
		if (availableRippleQuantities.IsEmpty() || availableRippleQuantities.Count() < 2)
		{
			SetUpRippleQuantityActionListeners();
			return;
		}

		int currentRippleQuantityId = availableRippleQuantities.Find(GetRippleQuantity());
		int desiredRippleQuantityId = currentRippleQuantityId + 1;
		if (desiredRippleQuantityId >= availableRippleQuantities.Count())
			desiredRippleQuantityId = 0;

		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(GetOwner());
		controller.ReplicateTurretFireModeChange(SCR_EFireModeChange.RIPPLE_QUANTITY, desiredRippleQuantityId, rplComp.Id())
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] controllingCharacter
	void SetUpAllActionListeners(notnull ChimeraCharacter controllingCharacter)
	{
		m_ControllingCharacter = controllingCharacter;
		SetUpWeaponGroupActionListeners();
		SetUpFireModeActionListeners();
		SetUpRippleQuantityActionListeners();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetUpWeaponGroupActionListeners()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		if (m_aWeaponGroups.Count() > 1)
			inputMgr.AddActionListener(ACTION_NAME_WEAPON_GROUP, EActionTrigger.DOWN, ReplicatedNextWeaponGroup);
		else
			inputMgr.RemoveActionListener(ACTION_NAME_WEAPON_GROUP, EActionTrigger.DOWN, ReplicatedNextWeaponGroup);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetUpFireModeActionListeners()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		array<int> intValues = {};
		if (SCR_Enum.BitToIntArray(m_aWeaponGroups[m_iCurrentWeaponGroup].m_eFireMode, intValues) > 1)
			inputMgr.AddActionListener(ACTION_NAME_FIRE_MODE, EActionTrigger.DOWN, ReplicatedNextFireMode);
		else
			inputMgr.RemoveActionListener(ACTION_NAME_FIRE_MODE, EActionTrigger.DOWN, ReplicatedNextFireMode);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetUpRippleQuantityActionListeners()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		if (m_eCurrentFireMode == EWeaponGroupFireMode.RIPPLE && m_aWeaponGroups[m_iCurrentWeaponGroup].m_aRippleFireQuantities.Count() > 1)
			inputMgr.AddActionListener(ACTION_NAME_RIPPLE_QUANTITY, EActionTrigger.DOWN, ReplicatedNextRippleQuantity);
		else
			inputMgr.RemoveActionListener(ACTION_NAME_RIPPLE_QUANTITY, EActionTrigger.DOWN, ReplicatedNextRippleQuantity);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void RemoveActionListeners()
	{
		m_ControllingCharacter = null;
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.RemoveActionListener(ACTION_NAME_WEAPON_GROUP, EActionTrigger.DOWN, ReplicatedNextWeaponGroup);
		inputMgr.RemoveActionListener(ACTION_NAME_FIRE_MODE, EActionTrigger.DOWN, ReplicatedNextFireMode);
		inputMgr.RemoveActionListener(ACTION_NAME_RIPPLE_QUANTITY, EActionTrigger.DOWN, ReplicatedNextRippleQuantity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
}
