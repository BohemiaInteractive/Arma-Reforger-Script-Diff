/*
Returns position for a character to avoid a vehicle.
The position is on the left or ride side of vehicle direction, depending on which side the character is.
*/
class SCR_AIGetCarAvoidPosition : AITaskScripted
{
	[Attribute("1", UIWidgets.EditBox)]
	protected float m_fAsideMoveDistance;
	
	protected static const string VEHICLE_PORT = "Vehicle";
	protected static const string POSITION_PORT = "Position";
	
	static override bool VisibleInPalette() { return true; }
	
	static override string GetOnHoverDescription()
	{
		return "Returns position for a character to avoid a vehicle.";
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity myEntity = owner.GetControlledEntity();
		
		IEntity vehicle;
		GetVariableIn(VEHICLE_PORT, vehicle);
		if (!vehicle || !myEntity)
			return ENodeResult.FAIL;
		
		// Get my pos in vehicle space
		vector myPos = myEntity.GetOrigin();
		vector myPosVehicleSpace = vehicle.CoordToLocal(myPos);
		
		// Vehicle's aside vector, Y component removed
		vector vehicleAside = vehicle.GetTransformAxis(0);
		vehicleAside[1] = 0;
		if (vehicleAside.LengthSq() < 0.01)
		{
			// If vehicle aside vector is not in XZ plane, probably vehicle is flipped sideways
			// In this case let's move away from vehicle
			
			vector vectorAwayFromVehicle = vector.Direction(vehicle.GetOrigin(), myPos);
			vectorAwayFromVehicle.Normalize();
			
			vector posOut = myPos + m_fAsideMoveDistance * vectorAwayFromVehicle;
			SetVariableOut(POSITION_PORT, posOut);
		}
		else
		{
			vehicleAside.Normalize();
			// Decide which direction we should move
			vector myOffsetDirection = vehicleAside; // Direction where we will move
			if (myPosVehicleSpace[0] < 0)	// I am left of vehicle
				myOffsetDirection = -myOffsetDirection;
			
			vector posOut = myPos + m_fAsideMoveDistance * myOffsetDirection;
			
			SetVariableOut(POSITION_PORT, posOut);
		}
		
		return ENodeResult.SUCCESS;
	}
	
	protected static ref TStringArray s_aVarsIn = { VEHICLE_PORT };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected static ref TStringArray s_aVarsOut = { POSITION_PORT };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
}