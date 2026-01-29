[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_FUELTANKClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_FUELTANK : SCR_BaseTutorialStage
{
	protected HitZone m_FuelTankHitZone;
	protected SCR_FuelManagerComponent m_FuelManager;
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
		
		m_FuelManager = SCR_FuelManagerComponent.Cast(veh.FindComponent(SCR_FuelManagerComponent));
		
		RegisterWaypoint(veh, "", "REPAIR");
		
		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 17);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_FuelManager)
			m_FuelManager.SetTotalFuelPercentage(0); //I'm not sure if this is wise idea, but let's keep it here for now.
	
		return m_FuelTankHitZone && m_FuelTankHitZone.GetHealth() > 20;
	}
};