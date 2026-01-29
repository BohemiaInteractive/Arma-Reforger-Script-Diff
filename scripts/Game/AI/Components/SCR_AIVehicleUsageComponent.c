//! Vehicle type from point view of AI usage
enum EAIVehicleType
{
	INVALID,				// Default value when not configured
	
	GROUND_VEHICLE_WHEELED,	// All wheeled ground vehicles, with weapon or not
	GROUND_VEHICLE_TRACKED,	// All tracked ground vehicles, with weapon or not
	
	STATIC_WEAPON,			// Static weapons which shoot directly, like static MGs
	STATIC_ARTILLERY,		// Static mortars
	
	AIRCRAFT_HELICOPTER,	// Helicopters
	AIRCRAFT_PLANE			// Planes
}

class SCR_AIVehicleUsageComponentClass : ScriptComponentClass
{
}

void SCR_AIOnVehicleDeleted(SCR_AIVehicleUsageComponent comp);
typedef func SCR_AIOnVehicleDeleted;

void SCR_AIOnVehicleDamageStateChanged(SCR_AIVehicleUsageComponent comp, EDamageState state);
typedef func SCR_AIOnVehicleDamageStateChanged;

//! This component should be attached to root entity of all vehicles and static turrets which are usable by AI.
//! In case of a static tripod inside composition, this component should be on static tripod though, not on composition root.
//! Other AI systems depend on this, so some AI functionality might not work with vehicles without this component.
class SCR_AIVehicleUsageComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.ComboBox, "Vehicle type from point view of AI usage", enums: ParamEnumArray.FromEnum(EAIVehicleType))]
	protected EAIVehicleType m_eVehicleType;
	
	[Attribute("1", UIWidgets.CheckBox, "Can be AI a pilot of this vehicle")]
	protected bool m_bCanBePiloted;
	
	protected ref ScriptInvokerBase<SCR_AIOnVehicleDeleted> m_OnDeleted;
	protected ref ScriptInvokerBase<SCR_AIOnVehicleDamageStateChanged> m_OnDamageStateChanged;
	protected TurretCompartmentSlot m_TurretSlot;
	protected PilotCompartmentSlot m_PilotSlot;
	protected SCR_DamageManagerComponent m_DamageManager;

	//------------------------------------------------------------------------------
	ScriptInvokerBase<SCR_AIOnVehicleDeleted> GetOnDeleted()
	{
		if (!m_OnDeleted)
			m_OnDeleted = new ScriptInvokerBase<SCR_AIOnVehicleDeleted>();
		
		return m_OnDeleted;
	}
	
	//------------------------------------------------------------------------------
	ScriptInvokerBase<SCR_AIOnVehicleDamageStateChanged> GetOnDamageStateChanged()
	{
		if (!m_OnDamageStateChanged)
			m_OnDamageStateChanged = new ScriptInvokerBase<SCR_AIOnVehicleDamageStateChanged>();
		
		return m_OnDamageStateChanged;
	}

	TurretCompartmentSlot GetTurretCompartmentSlot()
	{
		return m_TurretSlot;
	}

	PilotCompartmentSlot GetPilotCompartmentSlot()
	{
		return m_PilotSlot;
	}
	
	//------------------------------------------------------------------------------
	//! AI can use the pilot compartments on this vehicle
	bool CanBePiloted()
	{
		return m_bCanBePiloted;
	}
	
	//------------------------------------------------------------------------------
	EAIVehicleType GetVehicleType()
	{
		return m_eVehicleType;
	}
	
	//------------------------------------------------------------------------------
	bool IsVehicleTypeValid()
	{
		return m_eVehicleType != EAIVehicleType.INVALID;
	}

	//------------------------------------------------------------------------------------------------
	//! Determines whether the specified AI group is occupying this vehicle.
	//! \param[in] aiGroup The AI group to check for occupancy.
	//! \return Returns true if any member of the group is occupying the vehicle; otherwise, false.
	bool IsOccupiedByGroup(notnull SCR_AIGroup aiGroup)
	{
		array<AIAgent> members = {};
		aiGroup.GetAgents(members);
		foreach (AIAgent agent : members)
		{
			if (IsOccupiedBy(agent.GetControlledEntity()))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks whether the specified entity is occupying any compartment in this vehicle.
	//! \param[in] entity The entity to check for occupancy.
	//! \return Returns true if the entity is occupying a compartment; otherwise, false.
	bool IsOccupiedBy(IEntity entity)
	{
		SCR_BaseCompartmentManagerComponent compartmentMananager = SCR_BaseCompartmentManagerComponent.Cast(GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentMananager)
			return false;

		array<BaseCompartmentSlot> compartmentSlots = {};
		compartmentMananager.GetCompartments(compartmentSlots);
		foreach (BaseCompartmentSlot slot : compartmentSlots)
		{
			if (slot.GetOccupant() == entity)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	EDamageState GetDamageState()
	{
		if (!m_DamageManager)
			return EDamageState.UNDAMAGED;

		return m_DamageManager.GetState();
	}

	//---------------------------------------------------------------------------------------------------
	//! Finds the component on nearest parent of that entity, including that entity.
	//! This should help with vehicles which have benches as slots, or static weapons which are children of compositions.
	static SCR_AIVehicleUsageComponent FindOnNearestParent(notnull IEntity ent, out IEntity componentOwner)
	{
		SCR_AIVehicleUsageComponent comp = null;
		
		while (!comp && ent)
		{
			comp = SCR_AIVehicleUsageComponent.Cast(ent.FindComponent(SCR_AIVehicleUsageComponent));
			componentOwner = ent;
			ent = ent.GetParent();
		}
		
		return comp;
	}
		
	//---------------------------------------------------------------------------------------------------
	// PROTECTED / INTERNAL
	
	//------------------------------------------------------------------------------
	protected void OnDamageStateChanged(EDamageState damageState)
	{
		if (!m_OnDamageStateChanged)
			return;
		
		m_OnDamageStateChanged.Invoke(this, damageState);
	}
	
	//------------------------------------------------------------------------------
	protected void ~SCR_AIVehicleUsageComponent()
	{		
		if (m_DamageManager)
			m_DamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);

		if (m_OnDeleted)
			m_OnDeleted.Invoke(this);
	}
	
	//------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
		
		if (!IsVehicleTypeValid())
			ErrorIncorrectType(owner);
	}
	
	//------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		m_DamageManager = SCR_DamageManagerComponent.Cast(owner.FindComponent(SCR_DamageManagerComponent));
		if (m_DamageManager)
			m_DamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);

		// find pilot & turret slots
		SCR_BaseCompartmentManagerComponent compartmentMan = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (compartmentMan)
		{
			array<BaseCompartmentSlot> compartmentSlots = {};
			compartmentMan.GetCompartments(compartmentSlots);
			foreach (BaseCompartmentSlot slot : compartmentSlots)
			{
				TurretCompartmentSlot turretComp = TurretCompartmentSlot.Cast(slot);
				PilotCompartmentSlot pilotComp = PilotCompartmentSlot.Cast(slot);
				if (turretComp)
					m_TurretSlot = turretComp;
				if (pilotComp)
					m_PilotSlot = pilotComp;
			}
		}
	}
	
	//------------------------------------------------------------------------------
	static void ErrorNoComponent(notnull IEntity entity)
	{
		Print(string.Format("SCR_AIVehicleUsageComponent not found on entity: %1", entity), LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------
	static void ErrorIncorrectType(notnull IEntity entity)
	{
		Print(string.Format("SCR_AIVehicleUsageComponent has incorrect type on entity: %1", entity), LogLevel.ERROR);
	}
}
