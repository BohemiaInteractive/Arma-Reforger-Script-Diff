[BaseContainerProps()]
class SCR_ScenarioFrameworkVehicleActionDamageHitZoneByName : SCR_ScenarioFrameworkVehicleActionBase
{
	[Attribute(defvalue: "100", desc: "Health Percentage to be set for target hit zone", UIWidgets.Graph, "0 100 1")]
	int m_iHealthPercentage;
	
	[Attribute(defvalue: "", desc: "HitZone Name")]
	string m_sHitZoneName;
	
	protected HitZone m_HitZone;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	override void Init(Vehicle vehicle)
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkVehicleActionDamageHitZone.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(vehicle);
		if (!damageManager)
		{
			Print(string.Format("[SCR_ScenarioFrameworkVehicleActionDamageHitZone.Init] couldn't found damage manager for action", this), LogLevel.ERROR);
			return;
		}
		
		m_HitZone = damageManager.GetHitZoneByName(m_sHitZoneName);
		if (!m_HitZone)
			return;
		
		OnActivate();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		if (m_HitZone)
			m_HitZone.SetHealthScaled(m_iHealthPercentage * 0.01);
	}
}