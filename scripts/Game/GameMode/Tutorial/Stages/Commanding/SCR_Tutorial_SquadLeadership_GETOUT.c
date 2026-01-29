[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_GETOUTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_GETOUT : SCR_BaseTutorialStage
{
	ref array <BaseCompartmentSlot> m_Compartments;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity vehicle = GetGame().GetWorld().FindEntityByName("COMMANDING_JEEP");
		if (!vehicle)
			return;
		
		SCR_BaseCompartmentManagerComponent compartmentComp = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentComp)
			return;
		
		m_Compartments = {};
		compartmentComp.GetCompartments(m_Compartments);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		foreach (BaseCompartmentSlot compartment : m_Compartments)
		{
			if (compartment.GetOccupant() == m_Player)
				return false;
		}
		
		return true;
	}
}