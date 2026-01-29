[BaseContainerProps()]
class SCR_ScenarioFrameworkOnProjectileShotConditionWeaponPrefab : SCR_ScenarioFrameworkOnProjectileShotConditionBase
{
	[Attribute(desc: "Weapon Resource Name", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sWeaponResourceName;
	
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkOnProjectileShotConditionWeaponPrefab.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (!weapon)
			return false;
		
		IEntity weaponEnt = weapon.GetOwner();
		
		return weaponEnt && weaponEnt.GetPrefabData().GetPrefabName() == m_sWeaponResourceName;
	}
}