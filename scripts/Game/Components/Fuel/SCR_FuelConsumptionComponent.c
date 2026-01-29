[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "SCR_FuelConsumptionComponent", color: "0 0 255 255")]
class SCR_FuelConsumptionComponentClass : ScriptGameComponentClass
{
	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Automatic fuel tank switching. Automatically select next fuel tank upon depletion of current fuel tank." )]
	bool m_bAutomaticFuelTankSwitching;
	
	[Attribute( defvalue: "20", uiwidget: UIWidgets.Auto, desc: "Fuel consumption at max power RPM\n[liters/hour]" )]
	float m_fFuelConsumption;
	
	[Attribute( defvalue: "0.5", uiwidget: UIWidgets.Auto, desc: "Fuel consumption idle\n[liters/hour]" )]
	float m_fFuelConsumptionIdle;
}

class SCR_FuelConsumptionComponent : ScriptGameComponent
{
	protected const float							SECOND_TO_HOUR		= 1/3600;
	protected const float							MIN_FUEL			= 0;
	protected const float							MIN_THRUST			= 0.04;
	protected const float							MIN_CLUTCH			= 0.04;
	protected const float							NEUTRAL_GEAR		= 1;
	protected const float							TIME_STEP			= 1;
	
	protected bool									m_bConnectedToSystem = false;
	
	protected float									m_fTimeDelta;
	protected BaseFuelNode							m_CurrentFuelTank;
	protected VehicleBaseSimulation					m_Simulation;
	protected SCR_FuelConsumptionComponentClass		m_ComponentData;
	
	//! Global Fuel consumption scale for vehicles (and other fuel consumers). x1 means the fuel consumption is roughly equal to real world fuel consumption
	protected static float s_fGlobalFuelConsumptionScale = 8;
	
	//------------------------------------------------------------------------------------------------
	//! Set Fuel consumption scale of vehicles (and other fuel consumers) (Server only)
	//! \param[in] globalFuelConsumptionScale New fuel consumption scale
	//! \param[in] playerThatChangedValue Optional, if value given then a notification is send on scale changed
	static void SetGlobalFuelConsumptionScale(float globalFuelConsumptionScale, int playerThatChangedValue = -1)
	{
		if (s_fGlobalFuelConsumptionScale == globalFuelConsumptionScale || globalFuelConsumptionScale < 0)
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode || gameMode.IsMaster())
		{
			//~ Send notification when fuel consumption scale is changed and player Id is given
			if (playerThatChangedValue > 0)
				SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.EDITOR_CHANGED_FUEL_CONSUMPTION_SCALE, playerThatChangedValue, globalFuelConsumptionScale * 100);
		
			s_fGlobalFuelConsumptionScale = globalFuelConsumptionScale;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Global Fuel consumption Scale
	static float GetGlobalFuelConsumptionScale()
	{
		return s_fGlobalFuelConsumptionScale;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Switch the actual fuel tank (i.e. by a switch on the dashboard)
	//! \param[in] iFuelTankID
	void SetCurrentFuelTank(int iFuelTankID)
	{
		m_CurrentFuelTank = FindFuelTankByID(iFuelTankID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	BaseFuelNode GetCurrentFuelTank()
	{
		return m_CurrentFuelTank;
	}

	//------------------------------------------------------------------------------------------------
	//! returns the fuel tank pointer by the given FuelTankID
	//! \param[in] iFuelTankID
	//! \return
	protected BaseFuelNode FindFuelTankByID(int iFuelTankID)
	{
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(GetOwner().FindComponent(FuelManagerComponent));
		if (!fuelManager)
			return null;
		
		array<BaseFuelNode> nodes = {};
		fuelManager.GetFuelNodesList(nodes);
		SCR_FuelNode scrNode;
		foreach (BaseFuelNode node: nodes)
		{
			// Currently ID is limited to SCR_FuelNode
			scrNode = SCR_FuelNode.Cast(node);
			if (scrNode && scrNode.GetFuelTankID() == iFuelTankID)
				return scrNode;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the pointer of the first fuel tank with enough fuel (the fuel tank which is dedicated to the vehicle, not its cistern, cargo...)
	private BaseFuelNode FindNonEmptyFuelTank()
	{
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(GetOwner().FindComponent(FuelManagerComponent));
		if (!fuelManager)
			return null;
		
		array<BaseFuelNode> nodes = {};
		fuelManager.GetFuelNodesList(nodes);
		foreach (BaseFuelNode node: nodes)
		{
			if (node && node.GetFuel() > MIN_FUEL)
				return node;
		}
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] enabled
	void SetEnabled(bool enabled)
	{
		if (!m_ComponentData)
			return;
		
		// Irrelevant on proxies
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rpl && rpl.IsProxy())
			return;
		
		if (enabled && m_bConnectedToSystem)
			m_fTimeDelta = 0;
		
		if (m_Simulation)
			Update();
		else
			enabled = false;
		
		if (enabled)
			ConnectToFuelConsumptionSystem();
		else
			DisconnectFromFuelConsumptionSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	//! updates currently active fueltank to see if it can still be used. turns off engine if no usable fueltanks are present.
	void Update()
	{
		// if there is fuel in the actual tank
		if (!m_CurrentFuelTank || m_CurrentFuelTank.GetFuel() <= MIN_FUEL)
		{
			// automatic switching not disabled
			if (m_ComponentData.m_bAutomaticFuelTankSwitching)
				m_CurrentFuelTank = FindNonEmptyFuelTank();
			else
				m_CurrentFuelTank = null;
		}
		
		// vehicle has at least one fuel tank
		if (m_CurrentFuelTank)
		{
			float currentConsumption;

			VehicleWheeledSimulation wheeledSimulation = VehicleWheeledSimulation.Cast(m_Simulation);
			if (wheeledSimulation)
			{
				float throttle = wheeledSimulation.GetThrottle();
				if (throttle >= MIN_THRUST && wheeledSimulation.GetGear() != NEUTRAL_GEAR && wheeledSimulation.GetClutch() >= MIN_CLUTCH)
					currentConsumption = m_ComponentData.m_fFuelConsumption * throttle * wheeledSimulation.EngineGetRPM() / wheeledSimulation.EngineGetRPMPeakTorque();
				else if (throttle >= MIN_THRUST)
					currentConsumption = m_ComponentData.m_fFuelConsumptionIdle * wheeledSimulation.EngineGetRPM() / wheeledSimulation.EngineGetRPMIdle();
				else
					currentConsumption = m_ComponentData.m_fFuelConsumptionIdle;
			}

			VehicleHelicopterSimulation helicopterSimulation = VehicleHelicopterSimulation.Cast(m_Simulation);
			if (helicopterSimulation)
			{
				currentConsumption = m_ComponentData.m_fFuelConsumption;
			}

			// Scale and convert consumption rate per hour to per second
			float consumptionScale = m_fTimeDelta * s_fGlobalFuelConsumptionScale * SECOND_TO_HOUR;

			float newFuel = m_CurrentFuelTank.GetFuel() - currentConsumption * consumptionScale;
	
	#ifdef ENABLE_DIAG
			if (wheeledSimulation && currentConsumption > 0 && DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_VEHICLES_FUEL_CONSUMPTION))
			{
				float throttle = wheeledSimulation.GetThrottle();
				float speed = wheeledSimulation.GetSpeedKmh();
				float range;
				
				if (s_fGlobalFuelConsumptionScale != 0 && currentConsumption != 0)
					range = speed * newFuel / (currentConsumption * s_fGlobalFuelConsumptionScale);
				else 
					range = -1;
				
				float litersPer100KM;
				if (speed > 0)
					litersPer100KM = currentConsumption * 100 / speed;

				Print(throttle);
				Print(speed);
				Print(currentConsumption);
				Print(newFuel);
				Print(range);
				Print(litersPer100KM);
			}
	#endif // ENABLE_DIAG
			m_CurrentFuelTank.SetFuel(newFuel);
		}
		else
		{
			VehicleControllerComponent controller = VehicleControllerComponent.Cast(GetOwner().FindComponent(VehicleControllerComponent));
			if (controller)
			{
				controller.StopEngine(false);
			}
		}
		
		m_fTimeDelta = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ConnectToFuelConsumptionSystem()
	{
		World world = GetOwner().GetWorld();
		FuelConsumptionSystem updateSystem = FuelConsumptionSystem.Cast(world.FindSystem(FuelConsumptionSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Register(this);
		m_bConnectedToSystem = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromFuelConsumptionSystem()
	{
		World world = GetOwner().GetWorld();
		FuelConsumptionSystem updateSystem = FuelConsumptionSystem.Cast(world.FindSystem(FuelConsumptionSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Unregister(this);
		m_bConnectedToSystem = false;
	}
	
	//------------------------------------------------------------------------ COMMON METHODS ------------------------------------------------------------------------//
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] timeSlice
	void Update(float timeSlice)
	{
		m_fTimeDelta += timeSlice;
		if (m_fTimeDelta > TIME_STEP)
			Update();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_ComponentData = SCR_FuelConsumptionComponentClass.Cast(GetComponentData(owner));
		if (!m_ComponentData)
			return;
		
		// Irrelevant on proxies
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rpl && !rpl.IsProxy())
			SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(owner.FindComponent(FuelManagerComponent));
		if (!fuelManager)
			return;

		m_CurrentFuelTank = FindNonEmptyFuelTank();

		m_Simulation = VehicleBaseSimulation.Cast(owner.FindComponent(VehicleBaseSimulation));

		if (m_Simulation && !m_Simulation.IsValid())
			m_Simulation = null;
		
		if (!m_Simulation)
			return;

		VehicleWheeledSimulation wheeledSimulation = VehicleWheeledSimulation.Cast(m_Simulation);
		if (wheeledSimulation && wheeledSimulation.EngineIsOn())
			SetEnabled(true);
		
		VehicleHelicopterSimulation helicopterSimulation = VehicleHelicopterSimulation.Cast(m_Simulation);
		if (helicopterSimulation && helicopterSimulation.EngineIsOn())
			SetEnabled(true);
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_VEHICLES_FUEL_CONSUMPTION, "", "Fuel consumption", "Vehicles");
#endif // ENABLE_DIAG
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromFuelConsumptionSystem();
		
		super.OnDelete(owner);
	}
}
