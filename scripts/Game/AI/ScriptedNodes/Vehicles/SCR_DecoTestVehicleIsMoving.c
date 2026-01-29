class SCR_AIDecoTestVehicleIsMoving : DecoratorTestScripted
{
	protected const float MIN_SPEED_MPS = 0.5;
	protected const float MIN_SPEED_RADS = 0.5;	
	protected BaseVehicle m_vehicle;
	protected VehicleHelicopterSimulation m_heliSimulation;
	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!controlled)
			return false;
		
		// Iterate parents of entity until we find first Vehicle entity
		IEntity parent = controlled;
		BaseVehicle vehicle = BaseVehicle.Cast(controlled);
		bool isMoving, isRotating, isInAir;
		
		while (vehicle == null && parent != null)
		{
			parent = parent.GetParent();
			if (parent)
				vehicle = BaseVehicle.Cast(parent);
		}
		
		if (!vehicle)
			return false;
		
		if (m_vehicle != vehicle)
		{
			m_vehicle = vehicle;
			m_heliSimulation = VehicleHelicopterSimulation.Cast(vehicle.FindComponent(VehicleHelicopterSimulation));			
		}	
				
		Physics ph = m_vehicle.GetPhysics();
		if (!ph || !ph.IsActive())
			return false;
		
		isMoving = ph.GetVelocity().Length() > MIN_SPEED_MPS;
		isRotating = ph.GetAngularVelocity().Length() > MIN_SPEED_RADS;
		
		if (m_heliSimulation)
		{
			vector positionActual;
			positionActual = vehicle.GetOrigin();
			isInAir = !m_heliSimulation.HasAnyGroundContact();		
			return isInAir || isMoving || isRotating;
		}
		return isMoving || isRotating;
	}
};