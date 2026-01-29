[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_ENGINEFULLClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_ENGINEFULL : SCR_BaseTutorialStage
{
	protected HitZone m_EngineHitZone;
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
			m_EngineHitZone = damageManager.GetHitZoneByName("Engine_01");
		
		RegisterWaypoint(veh, "", "REPAIR");
		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 21);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_EngineHitZone && m_EngineHitZone.GetHealth() > 400;
	}
};