[ComponentEditorProps(category: "GameScripted/AI", description: "Component for AI checking state of characters")]
class SCR_AIInfoComponentClass : SCR_AIInfoBaseComponentClass
{
}

enum EUnitRole
{
	NONE 				= 0,
	RIFLEMAN 			= 1,
	MEDIC 				= 2,
	MACHINEGUNNER 		= 4,
	AT_SPECIALIST 		= 8,
	GRENADIER 			= 16,
	SNIPER				= 32,
	HAS_SMOKE_GRENADE	= 64,
	HAS_FRAG_GRENADE	= 128
}

enum EUnitState
{
	NONE 			= 0,
	WOUNDED 		= 1,
	IN_TURRET		= 2,
	IN_VEHICLE		= 4,
	PILOT			= 8,
	UNCONSCIOUS		= 16
}

enum EUnitAIState
{
	AVAILABLE,
	BUSY,
	UNRESPONSIVE,
}

void SCR_AIOnCompartmentEntered(AIAgent agent, IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move);
typedef func SCR_AIOnCompartmentEntered;

void SCR_AIOnCompartmentLeft(AIAgent agent, IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move);
typedef func SCR_AIOnCompartmentLeft;

void SCR_AIOnAgentLifeStateChanged(AIAgent agent, SCR_AIInfoComponent info, IEntity vehicle, ECharacterLifeState lifeState);
typedef func SCR_AIOnAgentLifeStateChanged;

class SCR_AIInfoComponent : SCR_AIInfoBaseComponent
{
	protected EUnitState m_iUnitStates;
	protected EUnitAIState m_iAIStates;	
	protected SCR_InventoryStorageManagerComponent m_inventoryManagerComponent;
	protected BaseWeaponManagerComponent m_weaponManagerComponent;
	protected SCR_CompartmentAccessComponent m_CompartmentAccessComponent;
	protected SCR_AIThreatSystem m_ThreatSystem;
	protected SCR_CharacterDamageManagerComponent m_DamageManager;
	protected SCR_AICombatComponent m_CombatComponent;
	protected SCR_CharacterControllerComponent m_CharacterController;
	PerceptionComponent m_Perception;
	
	ref ScriptInvokerBase<SCR_AIOnCompartmentEntered> m_OnCompartmentEntered = new ScriptInvokerBase<SCR_AIOnCompartmentEntered>();
	ref ScriptInvokerBase<SCR_AIOnCompartmentLeft> m_OnCompartmentLeft = new ScriptInvokerBase<SCR_AIOnCompartmentLeft>();
	ref ScriptInvokerBase<SCR_AIOnAgentLifeStateChanged> m_OnAgentLifeStateChanged = new ScriptInvokerBase<SCR_AIOnAgentLifeStateChanged>();
	
	protected SCR_CharacterBloodHitZone m_BloodHitZone;
	protected float m_fUnconsciousBloodLevel;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicle
	//! \param[in] manager
	//! \param[in] mgrID
	//! \param[in] slotID
	void OnVehicleEntered( IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		m_OnCompartmentEntered.Invoke(AIAgent.Cast(GetOwner()), vehicle, manager, mgrID, slotID, move);
		
		AddUnitState(EUnitState.IN_VEHICLE);
		
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		ECompartmentType compType = compSlot.GetType();
		if (compType == ECompartmentType.TURRET)
			AddUnitState(EUnitState.IN_TURRET);
		else if (compType == ECompartmentType.PILOT)
			AddUnitState(EUnitState.PILOT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicle
	//! \param[in] manager
	//! \param[in] mgrID
	//! \param[in] slotID
	void OnVehicleLeft( IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		m_OnCompartmentLeft.Invoke(AIAgent.Cast(GetOwner()), vehicle, manager, mgrID, slotID, move);
		
		RemoveUnitState(EUnitState.IN_VEHICLE);
		
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		ECompartmentType compType = compSlot.GetType();
		if (compType == ECompartmentType.TURRET)
			RemoveUnitState(EUnitState.IN_TURRET);
		else if (compType == ECompartmentType.PILOT)
			RemoveUnitState(EUnitState.PILOT);
	}
	

	//------------------------------------------------------------------------------------------------
	//!
	void InitBloodLevel()
	{
		m_BloodHitZone = m_DamageManager.GetBloodHitZone();
		if (m_BloodHitZone)
			m_fUnconsciousBloodLevel = m_BloodHitZone.GetMaxHealth() * m_BloodHitZone.GetDamageStateThreshold(EDamageState.STATE3);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		IEntity ent = owner;
		AIAgent agent = AIAgent.Cast(owner);
		if (agent)
			ent = agent.GetControlledEntity();
		
		if (ent)
		{
			m_inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(ent.FindComponent(SCR_InventoryStorageManagerComponent));
			m_weaponManagerComponent = BaseWeaponManagerComponent.Cast(ent.FindComponent(BaseWeaponManagerComponent));
			m_CompartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(ent.FindComponent(SCR_CompartmentAccessComponent));
			m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(ent.FindComponent(SCR_CharacterDamageManagerComponent));
			m_CombatComponent = SCR_AICombatComponent.Cast(ent.FindComponent(SCR_AICombatComponent));

			m_CharacterController = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
			if (m_CharacterController)
				m_CharacterController.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
			
			m_Perception = PerceptionComponent.Cast(ent.FindComponent(PerceptionComponent));
		}
		
		if (m_CompartmentAccessComponent)
		{
			m_CompartmentAccessComponent.GetOnCompartmentEntered().Insert(OnVehicleEntered);
			m_CompartmentAccessComponent.GetOnCompartmentLeft().Insert(OnVehicleLeft);
		}
		
		if (m_DamageManager)
		{
			InitBloodLevel();
			m_DamageManager.GetOnDamageEffectAdded().Insert(OnDamageEffectAdded);
			m_DamageManager.GetOnDamageEffectRemoved().Insert(OnDamageEffectRemoved);
			EvaluateWoundedState();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_AIInfoComponent()
	{
		if (m_CharacterController)
			m_CharacterController.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		if (m_CompartmentAccessComponent)
		{
			m_CompartmentAccessComponent.GetOnCompartmentEntered().Remove(OnVehicleEntered);
			m_CompartmentAccessComponent.GetOnCompartmentLeft().Remove(OnVehicleLeft);
		}
		
		if (m_DamageManager)
		{
			m_DamageManager.GetOnDamageEffectAdded().Remove(OnDamageEffectAdded);
			m_DamageManager.GetOnDamageEffectRemoved().Remove(OnDamageEffectRemoved);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AICombatComponent GetCombatComponent()
	{
		return m_CombatComponent;
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] agent
	//! \return
	bool IsOwnerAgent(AIAgent agent)
	{
		return GetOwner() == agent;
	}
	
//----------- BIT operations on Roles

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] role
	//! \return
	bool HasRole(EUnitRole role)
	{
		switch (role)
		{
			case EUnitRole.MEDIC:				return m_inventoryManagerComponent.GetHealthComponentCount() > 0;
			case EUnitRole.MACHINEGUNNER:		return m_CombatComponent.HasWeaponOfType(EWeaponType.WT_MACHINEGUN);
			case EUnitRole.RIFLEMAN:			return m_CombatComponent.HasWeaponOfType(EWeaponType.WT_RIFLE);
			case EUnitRole.AT_SPECIALIST:		return m_CombatComponent.HasWeaponOfType(EWeaponType.WT_ROCKETLAUNCHER);
			case EUnitRole.GRENADIER:			return m_CombatComponent.HasWeaponOfType(EWeaponType.WT_GRENADELAUNCHER); // todo right now it will not detect a UGL muzzle, because weapon type is still rifle
			case EUnitRole.SNIPER:			return m_CombatComponent.HasWeaponOfType(EWeaponType.WT_SNIPERRIFLE);
			case EUnitRole.HAS_SMOKE_GRENADE:	return m_CombatComponent.HasWeaponOfType(EWeaponType.WT_SMOKEGRENADE);
			case EUnitRole.HAS_FRAG_GRENADE:	return m_CombatComponent.HasWeaponOfType(EWeaponType.WT_FRAGGRENADE);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Use only for debugging!
	//! \return
	EUnitRole GetRoles()
	{
		typename t = EUnitRole;
		int tVarCount = t.GetVariableCount();
		EUnitRole roles = 0;
		for (int i = 0; i < tVarCount; i++)
		{
			EUnitRole flag;
			t.GetVariableValue(null, i, flag);
			if (flag && HasRole(flag))
				roles |= flag;
		}
		return roles;
	}
	
//---------- BIT operation on States

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] state
	void AddUnitState(EUnitState state)
	{
		m_iUnitStates = m_iUnitStates | state;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] state
	void RemoveUnitState(EUnitState state)
	{
		if (HasUnitState(state))
			m_iUnitStates = m_iUnitStates & ~state;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] state
	//! \return
	bool HasUnitState(EUnitState state)
	{
		return ( m_iUnitStates & state );
	}
	
	//------------------------------------------------------------------------------------------------
	//! Use only for debugging!
	//! \return
	EUnitState GetUnitStates()
	{
		return m_iUnitStates;
	}
	
//--------- AI states are disjoined - one can be in only one state at the time

	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetAIState(EUnitAIState state)
	{
		m_iAIStates = state; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EUnitAIState GetAIState()
	{
		return m_iAIStates;
	}
	
//--------  Info about magazines available to SCR_AIResupplyActivity
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] magazinyWellType
	//! \return
	int GetMagazineCountByWellType(typename magazinyWellType)
	{
		return m_CombatComponent.GetMagazineCount(magazinyWellType, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] threatSystem
	void InitThreatSystem(SCR_AIThreatSystem threatSystem)
	{
		m_ThreatSystem = threatSystem;	
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EAIThreatState GetThreatState()
	{
		if (m_ThreatSystem)
			return m_ThreatSystem.GetState();
		else 
			return EAIThreatState.SAFE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_AIThreatSystem GetThreatSystem()
	{
		return m_ThreatSystem;
	}
	
	//------------------------------------------------------------------------------------------------
	// Some obsolete functionality which we don't use
	
	[Obsolete()]
	void SetStance(ECharacterStance stance) {}
	
	[Obsolete()]
	ECharacterStance GetStance() { return 0; }
	
	[Obsolete()]
	void SetMovementType(EMovementType mode) {}
	
	[Obsolete()]
	EMovementType GetMovementType() { return 0; }
	
//-------- 	Evaluation of wounded state of AI

	//------------------------------------------------------------------------------------------------
	//! Returns true when state has changed and we must invoke an event
	protected void EvaluateWoundedState()
	{
		if (m_DamageManager.IsBleeding())
			AddUnitState(EUnitState.WOUNDED);
		else
			RemoveUnitState(EUnitState.WOUNDED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
	{
		if (dmgEffect.GetDamageType() != EDamageType.BLEEDING || !DotDamageEffect.Cast(dmgEffect))
			return;
		
		EvaluateWoundedState();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDamageEffectRemoved(notnull SCR_DamageEffect dmgEffect)
	{
		if (dmgEffect.GetDamageType() != EDamageType.BLEEDING || !DotDamageEffect.Cast(dmgEffect))
			return;
		
		EvaluateWoundedState();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] conscious
	void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState != ECharacterLifeState.INCAPACITATED)
			RemoveUnitState(EUnitState.UNCONSCIOUS);
		else
			AddUnitState(EUnitState.UNCONSCIOUS);
		
		AIAgent agent = AIAgent.Cast(GetOwner());
		if (!agent)
			return;
		
		IEntity vehicle;
		// find which vehicle this agent is in
		if (HasUnitState(EUnitState.IN_VEHICLE))
		{
			ChimeraCharacter ent = ChimeraCharacter.Cast(agent.GetControlledEntity());
			if (!ent)
				return;
			
			CompartmentAccessComponent compartComp = ent.GetCompartmentAccessComponent();
			if (!compartComp)
				return;
			
			vehicle = compartComp.GetVehicleIn(ent);
			
		}
		m_OnAgentLifeStateChanged.Invoke(agent, this, vehicle, newLifeState);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the time in seconds after which the agent loses consciousness due to bleeding or -1 if not bleeding
	float GetBleedTimeToUnconscious()
	{
		if (!m_BloodHitZone || !m_fUnconsciousBloodLevel)
			return -1;
		
		float bleedingPerSec = m_BloodHitZone.GetTotalBleedingAmount();
		float timeToUnconscious = -1;
		
		if (bleedingPerSec > 0)
			timeToUnconscious = (m_BloodHitZone.GetHealth() - m_fUnconsciousBloodLevel) / bleedingPerSec;
		
		return timeToUnconscious;
	}
	
	
//-------- Debugging

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] w
	// Used in SCR_AIDebugInfoComponent
	void DebugPrintToWidget(TextWidget w)
	{
		string str;
		str = str + string.Format("\n%1 %2", m_iAIStates, typename.EnumToString(EUnitAIState, m_iAIStates));
		w.SetText(str);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetBehaviorEditorDebugName()
	{
		AIAgent agent = AIAgent.Cast(GetOwner());
		
		SCR_CallsignCharacterComponent callsignComp = SCR_CallsignCharacterComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
		
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(agent.GetControlledEntity().FindComponent(FactionAffiliationComponent));
		
		string str;
		
		if (factionComp)
		{
			string faction = factionComp.GetAffiliatedFaction().GetFactionKey();
			str = str + string.Format("[%1] ", faction);
		}
		
		if (callsignComp)
		{
			string company, platoon, squad, character, format;
			bool setCallsign = callsignComp.GetCallsignNames(company, platoon, squad, character, format);
			if (setCallsign)
			{
				string callsign = WidgetManager.Translate(format, company, platoon, squad, character);
				str = str + string.Format(" %1", callsign);
			}
		}
		return str;
	}
}
