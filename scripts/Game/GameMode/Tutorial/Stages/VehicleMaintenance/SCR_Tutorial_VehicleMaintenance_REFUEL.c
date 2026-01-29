[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_REFUELClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_REFUEL : SCR_BaseTutorialStage
{
	protected SCR_FuelManagerComponent m_FuelManager;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		Vehicle veh = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("RepairingVehicle"));
		if (!veh)
			return;
		
		m_FuelManager = SCR_FuelManagerComponent.Cast(veh.FindComponent(SCR_FuelManagerComponent));
		
		RegisterWaypoint(veh, "", "FUEL");
		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 18);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_FuelManager && m_FuelManager.GetTotalFuel() > 2;
	}
};