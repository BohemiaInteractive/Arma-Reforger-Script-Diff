class SCR_AIFindAvailableVehicle: AITaskScripted
{
	static const string PORT_CENTER_OF_SEARCH		= "OriginIn";
	static const string PORT_RADIUS					= "RadiusIn";
	static const string PORT_VEHICLE_IN				= "VehicleIn";
	static const string PORT_VEHICLE_OUT			= "VehicleOut";
	static const string PORT_ROLE_OUT				= "RoleOut";
	static const string PORT_COMPARTMENT_OUT		= "CompartmentOut";
	static const string PORT_SEARCH_PARAMS			= "SearchParams";
	
	[Attribute("0", UIWidgets.CheckBox, "If found, add to list of usable vehicles?")]
	bool m_bAddToList;
	
	[Attribute("1", UIWidgets.CheckBox, "If found, reserve compartment?")]
	bool m_bReserveCompartment;
	
	private BaseWorld m_world;
	protected SCR_AIGroup m_group;
	private ref array<BaseCompartmentSlot> m_CompartmentSlots = {};
	protected IEntity m_VehicleToTestForCompartments;
	protected BaseCompartmentSlot m_Compartment;
	protected ECompartmentType m_CompartmentType;
	protected SCR_AIBoardingParameters m_WaypointParameter;
	protected SCR_AIGroupUtilityComponent m_groupUtilityCompoment;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_world = owner.GetWorld();
		m_group = SCR_AIGroup.Cast(owner);
		if (!m_group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
		}
		m_groupUtilityCompoment = m_group.GetGroupUtilityComponent();
		m_VehicleToTestForCompartments = null;
		m_Compartment = null;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_world || !m_group)
			return ENodeResult.FAIL;
		
		if (!m_VehicleToTestForCompartments && GetVariableIn(PORT_VEHICLE_IN, m_VehicleToTestForCompartments)) // reading the vehicle to test from outside of the node (another search)
			if (!GetVariableIn(PORT_SEARCH_PARAMS, m_WaypointParameter))
				return ENodeResult.FAIL;
		
		if (m_VehicleToTestForCompartments && !HasNoAvailableCompartment(m_VehicleToTestForCompartments)) // the vehicle was found already, does it have available compartment?
		{
			if (m_bReserveCompartment)
				m_group.AllocateCompartment(m_Compartment);
			SetVariablesOut(owner, m_VehicleToTestForCompartments, m_CompartmentType, m_Compartment);
			return ENodeResult.SUCCESS;
		}
		else								// perform new search for vehicle entity
		{ 
			m_VehicleToTestForCompartments = null;
			vector center;
			float radius;
			GetVariableIn(PORT_CENTER_OF_SEARCH, center);
			GetVariableIn(PORT_RADIUS, radius);
			GetVariableIn(PORT_SEARCH_PARAMS, m_WaypointParameter);
			
			if (!m_WaypointParameter) // waypoint can be deleted in the meantime
				return ENodeResult.FAIL;
			
			m_world.QueryEntitiesBySphere(center, radius, HasNoAvailableCompartment, FilterEntities, EQueryEntitiesFlags.DYNAMIC);
			if (m_VehicleToTestForCompartments)
			{
				if (m_bReserveCompartment)
					m_group.AllocateCompartment(m_Compartment);
				SetVariablesOut(owner, m_VehicleToTestForCompartments, m_CompartmentType, m_Compartment);
				return ENodeResult.SUCCESS;
			}
			ClearVariable(PORT_VEHICLE_OUT);
			ClearVariable(PORT_ROLE_OUT);
		};
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasNoAvailableCompartment(IEntity ent) 
	{
		BaseCompartmentManagerComponent compComp = BaseCompartmentManagerComponent.Cast(ent.FindComponent(BaseCompartmentManagerComponent));
		if (!compComp)
			return true;
		
		compComp.GetCompartments(m_CompartmentSlots);
		if (m_CompartmentSlots.IsEmpty())
			return true;
		
		SCR_AIVehicleUsageComponent vehicleUsage = SCR_AIVehicleUsageComponent.Cast(ent.FindComponent(SCR_AIVehicleUsageComponent));
		if (!vehicleUsage)
			return true;
		bool canBePiloted = vehicleUsage.CanBePiloted();
		BaseCompartmentSlot pilotCompartment, turretCompartment, cargoCompartment;
		
		foreach (BaseCompartmentSlot slot : m_CompartmentSlots)
		{
			if (slot.IsOccupied() || !slot.IsCompartmentAccessible() || slot.IsReserved())
				continue;
			if (m_WaypointParameter.m_bIsDriverAllowed && PilotCompartmentSlot.Cast(slot) && canBePiloted) //exclude helicopter pilot slots for now
				pilotCompartment = slot;
			else if (m_WaypointParameter.m_bIsGunnerAllowed && TurretCompartmentSlot.Cast(slot))
				turretCompartment = slot;
			else if (m_WaypointParameter.m_bIsCargoAllowed && CargoCompartmentSlot.Cast(slot))
				cargoCompartment = slot;
		}
		// going through priorities: pilot > turret > cargo
		if (pilotCompartment)
		{
			m_CompartmentType = ECompartmentType.PILOT;
			m_VehicleToTestForCompartments = ent;
			m_Compartment = pilotCompartment;
			return false;
		}
		
		if (turretCompartment)
		{
			m_CompartmentType = ECompartmentType.TURRET;
			m_VehicleToTestForCompartments = ent;
			m_Compartment = turretCompartment;
			return false;
		}
		
		if (cargoCompartment)
		{
			m_CompartmentType = ECompartmentType.CARGO;
			m_VehicleToTestForCompartments = ent;
			m_Compartment = cargoCompartment;
			return false;
		}
		return true; //continue search
	}
	
	//------------------------------------------------------------------------------------------------
	bool FilterEntities(IEntity ent) 
	{
		if (ent.FindComponent(BaseCompartmentManagerComponent) && (Turret.Cast(ent) || SCR_AIVehicleUsability.VehicleCanMove(ent)) && !SCR_AIVehicleUsability.VehicleIsOnFire(ent))
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVariablesOut(AIAgent owner, IEntity vehicleOut, ECompartmentType compartmentTypeOut, BaseCompartmentSlot compartment)
	{
		SetVariableOut(PORT_VEHICLE_OUT, vehicleOut);
		SetVariableOut(PORT_ROLE_OUT, compartmentTypeOut);
		SetVariableOut(PORT_COMPARTMENT_OUT, compartment);
		
		if (m_bAddToList)
		{
			SCR_AIGroup group = SCR_AIGroup.Cast(owner);
			if (group)
			{
				SCR_AIVehicleUsageComponent vehicleUsageComp = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicleOut, vehicleOut);
				if (vehicleUsageComp)
					group.GetGroupUtilityComponent().AddUsableVehicle(vehicleUsageComp);
			}
		}
	}	
		
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_VEHICLE_OUT,
		PORT_ROLE_OUT,
		PORT_COMPARTMENT_OUT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_CENTER_OF_SEARCH,
		PORT_RADIUS,
		PORT_SEARCH_PARAMS,
		PORT_VEHICLE_IN
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "SCR_AIFindAvailableVehicle: finds within radius of origin available compartments of vehicles with priority pilot>turret>cargo";
	}
};