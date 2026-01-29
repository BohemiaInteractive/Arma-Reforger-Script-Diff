[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_GETINClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_GETIN : SCR_BaseTutorialStage
{
	ref array <BaseCompartmentSlot> m_Compartments;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity vehicle = GetGame().GetWorld().FindEntityByName("PlayerVehicle");
		if (!vehicle)
			return;
		
		SCR_BaseCompartmentManagerComponent compartmentComp = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentComp)
			return;
		
		m_Compartments = {};
		compartmentComp.GetCompartments(m_Compartments);
		
		RegisterWaypoint(vehicle, "", "GETIN");
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 18);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{

		foreach (BaseCompartmentSlot compartment : m_Compartments)
		{
			if (compartment.GetOccupant() == m_Player)
				return true;
		}
		
		return false;
	}
}