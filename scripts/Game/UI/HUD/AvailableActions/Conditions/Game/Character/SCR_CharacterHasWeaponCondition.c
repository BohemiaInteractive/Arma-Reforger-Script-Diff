//! Returns true if character has weapon equipped
[BaseContainerProps()]
class SCR_CharacterHasWeaponCondition : SCR_AvailableActionCondition
{
	[Attribute("0", desc: "Check if character is using a weapon that is mounted on a turret")]
	protected bool m_bCheckTurretWeapon;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has a weapon equipped
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetCurrentWeaponEntity() != null || (m_bCheckTurretWeapon && data.GetCurrentTurretController() != null));
	}
}
