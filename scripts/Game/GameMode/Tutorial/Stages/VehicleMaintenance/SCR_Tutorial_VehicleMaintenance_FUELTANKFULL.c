[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_FUELTANKFULLClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_FUELTANKFULL : SCR_BaseTutorialStage
{
	protected HitZone m_FuelTankHitZone;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		Vehicle veh = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("RepairingVehicle"));
		if (!veh)
			return;
		 
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(veh.GetDamageManager());
		if (damageManager)
			m_FuelTankHitZone = damageManager.GetHitZoneByName("FuelTank_01");
		
		RegisterWaypoint(veh, "", "REPAIR");
		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 20);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_FuelTankHitZone && m_FuelTankHitZone.GetHealth() > 400;
	}
};