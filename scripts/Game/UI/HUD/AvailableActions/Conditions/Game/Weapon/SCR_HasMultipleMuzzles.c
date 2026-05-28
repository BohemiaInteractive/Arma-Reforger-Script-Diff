[BaseContainerProps()]
class SCR_HasMultipleMuzzles : SCR_AvailableActionCondition
{
	protected ref array<BaseMuzzleComponent> m_aMuzzles = {};

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		BaseWeaponComponent weapon = data.GetCurrentWeapon();
		return GetReturnResult(weapon && weapon.GetMuzzlesList(m_aMuzzles) > 1);
	}
}
