//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SetFireVehicleContextAction : SCR_SelectedEntitiesContextAction
{
	[Attribute(SCR_Enum.GetDefault(SCR_EBurningState.BURNING), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EBurningState))]
	protected SCR_EBurningState m_eTargetFireState;

	[Attribute(defvalue: "-1")]
	protected float m_fTargetFireStateWeight;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity.GetEntityType() == EEditableEntityType.VEHICLE;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(selectedEntity.GetOwner());
		if (!damageManager)
			return false;

		if (damageManager.GetState() == EDamageState.DESTROYED)
			return false;

		return (m_eTargetFireState == SCR_EBurningState.NONE) == damageManager.IsOnFire();
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(selectedEntity.GetOwner());
		if (!damageManager)
			return;

		if (damageManager.GetState() == EDamageState.DESTROYED)
			return;

		SCR_FlammableHitZone flammableHitZone = SCR_FlammableHitZone.Cast(damageManager.GetDefaultHitZone());
		if (!flammableHitZone)
			return;

		flammableHitZone.SetFireState(m_eTargetFireState, m_fTargetFireStateWeight);
		flammableHitZone.LockFireRate();
	}
};
