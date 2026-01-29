[BaseContainerProps()]
class SCR_ScenarioFrameworkVehicleActionDamageHitZonesByGroup : SCR_ScenarioFrameworkVehicleActionBase
{
	[Attribute(defvalue: "100", desc: "Health Percentage to be set for target hitzone", UIWidgets.Graph, "0 100 1")]
	int m_iHealthPercentage;
	
	[Attribute(defvalue: "0", desc: "Hit Zone Groups", uiwidget: UIWidgets.ComboBox, enumType: EVehicleHitZoneGroup)]
	ref array <EVehicleHitZoneGroup> m_aHitZoneGroups;
	
	protected ref array <HitZone> m_aHitZones = {};
	
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
		
		damageManager.GetHitZonesOfGroups(m_aHitZoneGroups, m_aHitZones);
		if (!m_aHitZones || m_aHitZones.IsEmpty())
			return;
		
		OnActivate();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		foreach (HitZone hitzone : m_aHitZones)
		{
			if (hitzone)
				hitzone.SetHealthScaled(m_iHealthPercentage * 0.01);	
		}
	}
}