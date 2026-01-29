[BaseContainerProps()]
class SCR_ScenarioFrameworkOnProjectileShotConditionWeaponType : SCR_ScenarioFrameworkOnProjectileShotConditionBase
{
	[Attribute(defvalue: "0", desc: "Weapon Type", uiwidget: UIWidgets.ComboBox, enumType: EWeaponType)]
	ref array <EWeaponType> m_aAllowedWeaponTypes;

	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkOnProjectileShotConditionWeaponType.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (!weapon)
			return false;
		
		return m_aAllowedWeaponTypes && m_aAllowedWeaponTypes.Contains(weapon.GetWeaponType());
	}
}