[BaseContainerProps()]
class SCR_GetOutGroupCommand : SCR_WaypointGroupCommand
{	
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		if (isClient && playerID == SCR_PlayerController.GetLocalPlayerId())
		{
			//SpawnWPVisualization(targetPosition, playerID);
			return true;
		}

		//SpawnWPVisualization(targetPosition, playerID);
		if (!m_sWaypointPrefab || !target || !targetPosition)
			return false;

		return GetOutAtVehiclePosition(target, playerID);
	}
	
	bool GetOutAtVehiclePosition(IEntity target, int playerID)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(target);
		if (!group)
			return false;
		
		array<SCR_ChimeraCharacter> aiMembers = {};
		aiMembers = group.GetAIMembers();
		if (aiMembers.IsEmpty())
			return false;
		
		CompartmentAccessComponent compartmentComp;
		IEntity vehicleIn;
		
		foreach (SCR_ChimeraCharacter character : aiMembers)
		{
			if (character && character.IsInVehicle())
			{
				compartmentComp = character.GetCompartmentAccessComponent();
				if (!compartmentComp)
					continue;
				
				vehicleIn = compartmentComp.GetVehicleIn(character);
				if (vehicleIn)
					break;
			}
		}
		
		if (!vehicleIn)
			return false;
		
		return SetWaypointForAIGroup(target, vehicleIn.GetOrigin(), playerID);
	}
}