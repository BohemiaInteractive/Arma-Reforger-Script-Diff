[BaseContainerProps()]
class SCR_ScenarioFrameworkOnProjectileShotConditionIsSuppressed : SCR_ScenarioFrameworkOnProjectileShotConditionBase
{
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkOnProjectileShotConditionIsSuppressed.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (!weapon)
			return false;
		
		BaseMuzzleComponent muzzle = weapon.GetCurrentMuzzle();
		if (muzzle)
			return muzzle.IsMuzzleSuppressed();
		
		return false;
	}
}