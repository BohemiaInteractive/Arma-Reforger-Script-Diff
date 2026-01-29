[BaseContainerProps()]
class SCR_ScenarioFrameworkOnProjectileShotConditionWeaponEntity : SCR_ScenarioFrameworkOnProjectileShotConditionBase
{
	[Attribute(desc: "SlotWaypoint that spawns waypoint")]
	ref SCR_ScenarioFrameworkGetLayerBase m_Getter;

	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkOnProjectileShotConditionWeaponEntity.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (!weapon)
			return false;
		
		if (!m_Getter)
		{
			PrintFormat("SCR_ScenarioFrameworkOnProjectileShotConditionWeaponEntity: Getter not found for %1", this, level:LogLevel.ERROR);
			return false;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			PrintFormat("SCR_ScenarioFrameworkOnProjectileShotConditionWeaponEntity: Issue with getter for %1", this, level:LogLevel.ERROR);
			return false;
		}

		return weapon.GetOwner() == entityWrapper.GetValue();
	}
}