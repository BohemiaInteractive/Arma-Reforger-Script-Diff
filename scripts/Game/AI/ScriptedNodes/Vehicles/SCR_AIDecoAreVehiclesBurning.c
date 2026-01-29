class SCR_AIDecoAreVehiclesBurning : DecoratorScripted
{
	static const string VEHICLE_PORT = "InEntity";
	static const string BURNING_VEHICLE_PORT = "BurningVehicleOut";
		
	//-----------------------------------------------------------------------------------------------------
	override bool TestFunction(AIAgent owner)
	{
		IEntity singleVehicle;
		SCR_DamageManagerComponent damageManager;
		GetVariableIn(VEHICLE_PORT, singleVehicle);
		if (singleVehicle)
		{
			BaseVehicle vehicle = BaseVehicle.Cast(singleVehicle);
			if (!vehicle)
				return false;
			damageManager = vehicle.GetDamageManager();
			if (!damageManager)
				return false;
			bool isOnFire = SCR_AIVehicleUsability.VehicleIsOnFire(singleVehicle, damageManager);
			return ReturnVariables(isOnFire, singleVehicle);
		}
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
			return false;
		
		
		SCR_AIGroupVehicleManager vehMgr = group.GetGroupUtilityComponent().m_VehicleMgr;
		
		array<ref SCR_AIGroupVehicle> usableVehicles = {};
		vehMgr.GetAllVehicles(usableVehicles);
		
		foreach (SCR_AIGroupVehicle groupVehicle: usableVehicles)
		{
			IEntity vehicleEntity = groupVehicle.GetEntity();
			BaseVehicle vehicle = BaseVehicle.Cast(vehicleEntity);
			if (!vehicle)
				continue;
			damageManager = vehicle.GetDamageManager();
			if (!damageManager)
				continue;
			bool isOnFire = SCR_AIVehicleUsability.VehicleIsOnFire(vehicleEntity, damageManager);
			if (isOnFire)
				return ReturnVariables(isOnFire, vehicleEntity);
		}
		
		
		ClearVariable(BURNING_VEHICLE_PORT);
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected bool ReturnVariables(bool isVehicleBurning, IEntity vehicle)
	{
		if (isVehicleBurning)
			SetVariableOut(BURNING_VEHICLE_PORT, vehicle);
		else
			ClearVariable(BURNING_VEHICLE_PORT);
		return isVehicleBurning;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		VEHICLE_PORT
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		BURNING_VEHICLE_PORT
	};
	protected override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//-----------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "DecoAreVehiclesBurning: if provided with vehicle entity, returns if it is burning, otherwise checks all known vehicles of group and returns (first) burning vehicle.";
	}
}
