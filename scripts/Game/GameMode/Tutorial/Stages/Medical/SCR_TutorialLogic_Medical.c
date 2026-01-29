[BaseContainerProps()]
class SCR_TutorialLogic_Medical : SCR_BaseTutorialCourseLogic
{
	protected SCR_CharacterDamageManagerComponent m_VictimDamageManager;
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return;
		
		m_VictimDamageManager = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!m_VictimDamageManager)
			return;
		
		//prepare injuries on victim
		m_VictimDamageManager.AddParticularBleeding("LThigh");
		m_VictimDamageManager.AddParticularBleeding("Chest");
		m_VictimDamageManager.GetHitZoneByName("LThigh").SetHealth(0);
		m_VictimDamageManager.GetHitZoneByName("Chest").SetHealth(0);
		m_VictimDamageManager.ForceUnconsciousness();
		
		GetGame().GetCallqueue().Remove(RefreshVictimBloodLevel);
		GetGame().GetCallqueue().Remove(RefreshVictimResilience);
		GetGame().GetCallqueue().CallLater(RefreshVictimBloodLevel, 1000, true);
		GetGame().GetCallqueue().CallLater(RefreshVictimResilience, 1000, true);
		
		IEntity vehicle = GetGame().GetWorld().FindEntityByName("AccidentJeep");
		if (!vehicle)
			return;
		
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(victim.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccess)
			return;
		
		//Lock jeep to prevent players from driving away with it.
		SCR_BaseLockComponent spawnProtectionComponent = SCR_BaseLockComponent.Cast(vehicle.FindComponent(SCR_BaseLockComponent));
		if (spawnProtectionComponent)
			spawnProtectionComponent.SetLocked(true);
		
		compartmentAccess.MoveInVehicle(vehicle, ECompartmentType.PILOT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{
		GetGame().GetCallqueue().Remove(RefreshVictimBloodLevel);
		GetGame().GetCallqueue().Remove(RefreshVictimResilience);
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		tutorial.InsertIntoGarbage("AccidentJeep");
		tutorial.InsertIntoGarbage("Victim");
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void RefreshVictimResilience()
	{
		if (!m_VictimDamageManager)
		{
			GetGame().GetCallqueue().Remove(RefreshVictimResilience);
			return;
		}
		
		m_VictimDamageManager.GetResilienceHitZone().SetHealth(0);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void RefreshVictimBloodLevel()
	{
		if (!m_VictimDamageManager)
		{
			GetGame().GetCallqueue().Remove(RefreshVictimResilience);
			return;
		}
		
		m_VictimDamageManager.GetBloodHitZone().SetHealth(2000);
	}
}