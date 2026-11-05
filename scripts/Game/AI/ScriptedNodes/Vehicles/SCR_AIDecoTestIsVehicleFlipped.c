class SCR_AIDecoTestIsVehicleFlipped: DecoratorTestScripted
{
	override protected bool TestFunction(AIAgent agent, IEntity controlled)
	{
		bool result;
		result = controlled && SCR_AIVehicleUsability.VehicleCanMove(controlled) 
				&& !SCR_AIVehicleUsability.VehicleIsOnFire(controlled) && SCR_AIVehicleUsability.IsVehicleFlipped(controlled);
		
		return result;
	}
};