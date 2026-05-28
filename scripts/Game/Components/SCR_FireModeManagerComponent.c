//------------------------------------------------------------------------------------------------
void OnTurretFireModeValuesChanged(EWeaponGroupFireMode fireMode, int quantity, int weaponGroupId);
typedef func OnTurretFireModeValuesChanged;
typedef ScriptInvokerBase<OnTurretFireModeValuesChanged> OnTurretFireModeValuesChangedInvoker;

class SCR_FireModeManagerComponentClass : FireModeManagerComponentClass 
{
}

class SCR_FireModeManagerComponent : FireModeManagerComponent
{
	protected ChimeraCharacter m_ControllingCharacter;
	protected ref OnTurretFireModeValuesChangedInvoker m_OnTurretFireModeValuesChanged;
	
	protected int m_iCurrentWeaponGroup;
	protected int m_iCurrentRippleIndex;
	
	EWeaponGroupFireMode m_eFireMode;
	protected int m_iRippleQuantity = 0;
	protected float m_fRippleInterval = 100.0;
	
	protected const	string EVENT_NAME_ENTER_COMPARTMENT = "OnCompartmentEntered";

	protected const string ACTION_NAME_RIPPLE_QUANTITY = "TurretWeaponNextRippleQuantity";
	protected const string ACTION_NAME_FIRE_MODE = "TurretWeaponNextFireMode";
	protected const string ACTION_NAME_WEAPON_GROUP = "TurretNextWeapon";
	
	//------------------------------------------------------------------------------------------------
	//! \return
	OnTurretFireModeValuesChangedInvoker GetOnTurretFireModeValuesChanged()
	{
		if (!m_OnTurretFireModeValuesChanged)
			m_OnTurretFireModeValuesChanged = new OnTurretFireModeValuesChangedInvoker();

		return m_OnTurretFireModeValuesChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponGroupId
	protected override void  OnWeaponsGroupChanged(int weaponsGroupIndex, int rippleModeIndex)
	{
		m_iCurrentWeaponGroup = weaponsGroupIndex;
		m_iCurrentRippleIndex = rippleModeIndex;
		
		FireModeManagerComponentClass prefab = GetPrefabData();
		
		array<int> ripples = {};
		if (prefab.GetAvailableRippleQuantities(m_iCurrentWeaponGroup, ripples))
		{
			m_iRippleQuantity = ripples[rippleModeIndex];
		}
		m_eFireMode = prefab.GetFireMode(m_iCurrentWeaponGroup);
		
		if (m_OnTurretFireModeValuesChanged)
			m_OnTurretFireModeValuesChanged.Invoke(m_eFireMode, m_iRippleQuantity, m_iCurrentWeaponGroup);
	}

	//------------------------------------------------------------------------------------------------
	FireModeManagerComponentClass GetPrefabData()
	{
		IEntity owner = GetOwner();
		
		EntityPrefabData prefab = owner.GetPrefabData();
		
		return FireModeManagerComponentClass.Cast(prefab.FindComponentData(FireModeManagerComponentClass));
	}
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNumberOfAvailableWeaponGroups()
	{
		FireModeManagerComponentClass prefab = GetPrefabData();
		return prefab.GetNumberOfWeaponsGroups();
	}
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNumberOfAvailableRippleQuantities()
	{
		FireModeManagerComponentClass prefab = GetPrefabData();
		return prefab.GetNumberOfRippleCounts(m_iCurrentWeaponGroup);
	}
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNumberOfAvailableFireModes()
	{
		FireModeManagerComponentClass prefab = GetPrefabData();
		array<int> ints = {};
		return SCR_Enum.BitToIntArray(prefab.GetFireMode(m_iCurrentWeaponGroup), ints);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] availableFireModes
	void GetAvailableFireModes(notnull out array<int> availableFireModes)
	{
		availableFireModes.Clear();
		FireModeManagerComponentClass prefab = GetPrefabData();
		prefab.GetAvailableFireModes(availableFireModes);
	}	
	
	//------------------------------------------------------------------------------------------------
	int GetAvailableRippleQuantities(notnull out array<int> availableRippleQuantities)
	{
		availableRippleQuantities.Clear();
		FireModeManagerComponentClass prefab = GetPrefabData();
		return prefab.GetAvailableRippleQuantities(m_iCurrentWeaponGroup, availableRippleQuantities);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] name
	//! \return
	int GetWeaponGroupID(out string name)
	{
		FireModeManagerComponentClass prefab = GetPrefabData();
		if (prefab)
			name = prefab.GetWeaponsGroupName(m_iCurrentWeaponGroup);
		
		return m_iCurrentWeaponGroup;
	}	
	
	//------------------------------------------------------------------------------------------------
	
	int GetCurrentWeaponIndices(notnull out array<int> weaponIndices)
	{
		weaponIndices.Clear();
		FireModeManagerComponentClass prefab = GetPrefabData();
		return prefab.GetWeaponIndices(m_iCurrentWeaponGroup, weaponIndices);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] name
	//! \return
	EWeaponGroupFireMode GetFireMode(out string name = string.Empty)
	{
		name = SCR_Enum.GetEnumName(EWeaponGroupFireMode, m_eFireMode);
		return m_eFireMode;
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

		NextWeaponsGroup();
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

		NextFireMode();
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

		NextRippleCount();
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
	void SetRippleInterval(float value)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetUpWeaponGroupActionListeners()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.AddActionListener(ACTION_NAME_WEAPON_GROUP, EActionTrigger.DOWN, ReplicatedNextWeaponGroup);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetUpFireModeActionListeners()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		array<int> intValues = {};
		inputMgr.AddActionListener(ACTION_NAME_FIRE_MODE, EActionTrigger.DOWN, ReplicatedNextFireMode);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetUpRippleQuantityActionListeners()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.AddActionListener(ACTION_NAME_RIPPLE_QUANTITY, EActionTrigger.DOWN, ReplicatedNextRippleQuantity);
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
	
}
