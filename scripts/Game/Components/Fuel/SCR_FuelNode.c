//~ Incoming liters per minute
enum EFuelFlowCapacityIn
{
	MANUAL 				= 	50,
	VEHICLE_SMALL 		= 	250,			//standard rate
	VEHICLE_MEDIUM		= 	700,			//usually trucks rate
	VEHICLE_BIG 		= 	1400,			//airplanes
	VEHICLE_HELICOPTER 	= 	2500,			//helicopter
	FUEL_CARGO 			= 	4500,			//Fuel Cargo like fuel tanks on truck
}

//~ Outgoing liters per minute
enum EFuelFlowCapacityOut
{
	MANUAL 				= 	50,
	VEHICLE_SMALL 		= 	250,			//standard rate
	VEHICLE_MEDIUM 		=	700,			//usually trucks rate
	VEHICLE_BIG 		= 	1400,			//airplanes
	VEHICLE_HELICOPTER 	= 	2500,			//helicopter
	FUEL_CARGO 			= 	4500,			//Fuel Cargo like fuel tanks on truck
}

//~ Fuel node type flags
enum SCR_EFuelNodeTypeFlag
{
	CAN_RECEIVE_FUEL = 1 << 0, ///< Can the fuel node receive fuel from other fuel nodes
	CAN_PROVIDE_FUEL = 1 << 1, ///< Can the fuel node provide fuel to other fuel nodes
	CAN_BE_DRAINED = 1 << 2, ///< If the fuel node can be drained with user action
	
	IS_FUEL_STORAGE = 1 << 3, ///< Any fuel nodes such as fuel tanker and in world refuel points
}

class SCR_FuelNode : BaseFuelNode
{
	[Attribute( defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Initial fuel level" )]
	protected float m_fInitialFuelTankState;	//tank state of the entity after it's created

	[Attribute(defvalue: EFuelFlowCapacityOut.MANUAL.ToString(), uiwidget: UIWidgets.SearchComboBox, desc: "Maximum Flow Capacity out eg: When draining the fuel tank to refuel another vehicle. Liters per minutes", enums: ParamEnumArray.FromEnum( EFuelFlowCapacityOut ) )]
	protected EFuelFlowCapacityOut m_MaxFlowCapacityOut;
	
	[Attribute(defvalue: EFuelFlowCapacityIn.VEHICLE_SMALL.ToString(), uiwidget: UIWidgets.SearchComboBox, desc: "Maximum Flow Capacity in eg: When filling the fuel tank. Liters per minutes", enums: ParamEnumArray.FromEnum( EFuelFlowCapacityIn ) )]
	protected EFuelFlowCapacityIn m_MaxFlowCapacityIn;

	[Attribute( defvalue: "20", uiwidget: UIWidgets.CheckBox, desc: "Maximum Leak Speed\n[l/min]" )]
	protected int m_iFuelLeakSpeed;
	
	[Attribute(SCR_EFuelNodeTypeFlag.CAN_RECEIVE_FUEL.ToString(), desc: "What type of fuel node is it? Can it provide and receive fuel? Maybe the fuel node is part of a fuel storage etc", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EFuelNodeTypeFlag))]
	protected SCR_EFuelNodeTypeFlag m_eFuelNodeType;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Fuel tank ID (user action and hitzone)" )]
	protected int m_iFuelTankID;	//for pairing with the user action

	static ref ScriptInvoker s_OnRefuelingFinished = new ScriptInvoker();
	protected ref ScriptInvokerFloat m_OnFuelChanged;

	#ifndef DISABLE_FUEL
	protected const float			TIME_STEP				= 1;
	protected float					m_fTimeSkip;											// Just for not overloading the onFrame each frame
	protected IEntity				m_Owner;												// Parent entity
	protected float					m_fHealth				= 1;							// Damage of fuel tank, reduces capacity

	protected SignalsManagerComponent m_SignalManagerComp;

	protected int					m_iSignalFuelStateIdx;
	protected int					m_iSignalIndexFuelTank;
	protected string				m_sSignalFuelState;
	protected const string			SIGNAL_FUEL_TANK_PREFIX				= "fuel";
	protected const string			SIGNAL_FUEL_TANK_ID					= "fueltank";

	#ifdef DEBUG_FUELSYSTEM
		private float 				m_fTestFuel = 0.0;
		private float 				m_fTestWorldTime = 0.0;
		[RplProp(condition: RplCondition.NoOwner, onRplName: "OnTestChanged")]
		private float				m_fDebugTankState 		= 0.0;
		private bool 				m_bFuelTankHUDAllowed	= true;
	#endif
	
	//------------------------------------------------------------------------------------------------
	//! \return true if fuel node can receive fuel
	bool CanReceiveFuel()
	{
		return SCR_Enum.HasFlag(m_eFuelNodeType, SCR_EFuelNodeTypeFlag.CAN_RECEIVE_FUEL);
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if fuel node can provide fuel
	bool CanProvideFuel()
	{
		return SCR_Enum.HasFlag(m_eFuelNodeType, SCR_EFuelNodeTypeFlag.CAN_PROVIDE_FUEL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if FuelNodeType enum is exactly the same as the nodeType enum
	//! \param typeFlag Queried flag types
	//! \return true if FuelNode Type contains the exact same flags, false otherwise
	bool HasExactTypeFlags(SCR_EFuelNodeTypeFlag typeFlag)
	{
		return typeFlag == m_eFuelNodeType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if FuelNodeType enum contains all the given fuel node type flags
	//! \param typeFlag Queried flag types
	//! \return true if FuelNode Type contains all required type flags
	bool HasAllTypeFlags(SCR_EFuelNodeTypeFlag typeFlag)
	{
		return SCR_Enum.HasFlag(m_eFuelNodeType, typeFlag);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if FuelNodeType enum contains any of the given fuel node type flags
	//! \param typeFlag Queried flag types
	//! \return True if FuelNode Type contains any required type flag
	bool HasAnyTypeFlag(SCR_EFuelNodeTypeFlag typeFlag)
	{
		return SCR_Enum.HasPartialFlag(m_eFuelNodeType, typeFlag);
	}

	//------------------------------------------------------------------------------------------------
	//! Assigned fuel tank ID
	int GetFuelTankID()
	{
		return m_iFuelTankID;
	}

	//------------------------------------------------------------------------------------------------
	//! Owner entity of the fuel tank
	IEntity GetOwner()
	{
		return m_Owner;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] health
	void SetHealth(float health)
	{
		m_fHealth = Math.Clamp(health, 0, 1);

		if (m_fHealth == 1)
			return;

		if (ShouldSimulate())
			return;

		SetShouldSimulate(true);
		m_fTimeSkip = 0;
	}

	//------------------------------------------------------------------------------------------------
	override void OnFixedFrame(IEntity owner, float timeSlice)
	{
		// Work 1 time per second at most
		float leakableFuel;
		if (m_fHealth < 1)
			leakableFuel = GetLeakableFuel();

		if (leakableFuel > 0)
			m_fTimeSkip += timeSlice;
		else
			m_fTimeSkip = 0;

		if (m_fTimeSkip < TIME_STEP)
			return;

		// Reduce capacity of fuel tank by its damage
		// The fuel over reliable capacity will leak gradually
		float leak = Math.Min(leakableFuel, ((1 - m_fHealth) * m_iFuelLeakSpeed / 60) * m_fTimeSkip);

		SetFuel(GetFuel() - leak);

		m_fTimeSkip = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnFuelChanged(float newFuel)
	{
		if (m_SignalManagerComp)
			m_SignalManagerComp.SetSignalValue(m_iSignalFuelStateIdx, newFuel);
		
		if (m_OnFuelChanged)
			m_OnFuelChanged.Invoke(newFuel);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerFloat GetOnFuelChanged()
	{
		if (!m_OnFuelChanged)
			m_OnFuelChanged = new ScriptInvokerFloat();
		
		return m_OnFuelChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetHealth()
	{
		return m_fHealth;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetLeakableFuel()
	{
		return GetFuel() - GetMaxFuel() * m_fHealth;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EFuelFlowCapacityOut GetMaxFlowCapacityOut()
	{
		return m_MaxFlowCapacityOut;
	}
		
	//------------------------------------------------------------------------------------------------
	float GetInitialFuelTankState()
	{
		return m_fInitialFuelTankState;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	EFuelFlowCapacityIn GetMaxFlowCapacityIn()
	{
		return m_MaxFlowCapacityIn;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		m_Owner = owner;

		m_sSignalFuelState = SIGNAL_FUEL_TANK_PREFIX + GetFuelTankID().ToString();
		#ifdef DEBUG_FUELSYSTEM
			PrintFormat( "FUELSYSTEM: Inserting %1", this );
		#endif

		m_SignalManagerComp = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		if (m_SignalManagerComp)
		{
			m_iSignalFuelStateIdx = m_SignalManagerComp.AddOrFindSignal(m_sSignalFuelState);
			
			#ifdef DEBUG_FUELSYSTEM
				PrintFormat( "FUELSYSTEM: signal registered: %1 and set to: %2", m_sSignalFuelState, m_SignalManagerComp.GetSignalValue( m_iSignalFuelStateIdx ) );
			#endif
		}
		
		SetFuel(m_fInitialFuelTankState);
	}

	#else

	//------------------------------------------------------------------------------------------------
	//! \return
	// Keeping just the declarations of function when the system is disabled
	bool CanReceiveFuel();

	//------------------------------------------------------------------------------------------------
	//! \return
	bool CanProvideFuel();

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetFuelTankID();

	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetOwner();

	//------------------------------------------------------------------------------------------------
	//! \param[in] health
	void SetHealth(float health);

	//------------------------------------------------------------------------------------------------
	override void OnFixedFrame(IEntity owner, float timeSlice);

	//------------------------------------------------------------------------------------------------
	protected void OnFuelChanged();

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetLeakableFuel();

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner);
	#endif
}
