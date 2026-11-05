class SCR_AIFindFlippedVehicle : AITaskScripted
{
	static const string PORT_CENTER_OF_SEARCH		= "OriginIn";
	static const string PORT_RADIUS					= "RadiusIn";
	static const string PORT_VEHICLE_IN				= "VehicleIn";
	static const string PORT_VEHICLE_OUT			= "VehicleOut";
	static const float UPDATE_INTERVAL_S			= 0.2;	
	
	[Attribute("0", UIWidgets.CheckBox, "Search periodically?")]
	bool m_bCanBeRunning;
	
	private BaseWorld m_World;
	protected IEntity m_FoundVehicle;
	protected float m_fUpdateTimer;
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_World = owner.GetWorld();
		m_fUpdateTimer = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_World)
			return ENodeResult.FAIL;
		
		if (m_bCanBeRunning) 
		{
			if (m_fUpdateTimer < UPDATE_INTERVAL_S)
			{
				m_fUpdateTimer += dt;
				return ENodeResult.RUNNING;
			}
			m_fUpdateTimer = 0;	
		}
		
		vector center;
		float radius;
		
		GetVariableIn(PORT_CENTER_OF_SEARCH, center);
		GetVariableIn(PORT_RADIUS, radius);
		GetVariableIn(PORT_VEHICLE_IN, m_FoundVehicle);
		
		ENodeResult res = ENodeResult.FAIL;
		
		if (IsVehicleValid(m_FoundVehicle))
			res = ENodeResult.SUCCESS;
		else
			m_FoundVehicle = null;
		
		if (!m_FoundVehicle)
 			m_World.QueryEntitiesBySphere(center, radius, CheckForVehicle, FilterEntities, EQueryEntitiesFlags.DYNAMIC);
		
		if (m_FoundVehicle)
		{
			SetVariableOut(PORT_VEHICLE_OUT, m_FoundVehicle);
			res = ENodeResult.SUCCESS;
		}	
		else
		{
			ClearVariable(PORT_VEHICLE_OUT);
			if (m_bCanBeRunning)
				res = ENodeResult.RUNNING;
			else
				res = ENodeResult.FAIL;
		}
		
		return res;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsVehicleValid(IEntity vehicleEntity)
	{
		return vehicleEntity && SCR_AIVehicleUsability.VehicleCanMove(vehicleEntity) 
				&& !SCR_AIVehicleUsability.VehicleIsOnFire(vehicleEntity) && SCR_AIVehicleUsability.IsVehicleFlipped(vehicleEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CheckForVehicle(IEntity ent) 
	{	
		if (IsVehicleValid(ent) && ent.FindComponent(BaseCompartmentManagerComponent))
		{
			m_FoundVehicle = ent;
			return false; // stops search
		}
		
		return true; //continue search	
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool FilterEntities(IEntity ent) 
	{
		if (Vehicle.Cast(ent))
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_VEHICLE_OUT
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_CENTER_OF_SEARCH,
		PORT_RADIUS,
		PORT_VEHICLE_IN
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	override static bool CanReturnRunning()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override static event protected bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override static event protected string GetOnHoverDescription()
	{
		return "SCR_AIFindFlippedVehicle: finds within radius of origin";
	}
};