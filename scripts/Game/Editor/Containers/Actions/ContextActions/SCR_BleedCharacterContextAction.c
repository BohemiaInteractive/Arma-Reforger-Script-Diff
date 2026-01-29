//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_BleedCharacterContextAction : SCR_SelectedEntitiesContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity.GetEntityType() == EEditableEntityType.CHARACTER;
	}

	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(SCR_CharacterDamageManagerComponent));
		if (!characterDamageManager)
			return false;

		if (characterDamageManager.GetState() == EDamageState.DESTROYED || characterDamageManager.IsBleeding())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(SCR_CharacterDamageManagerComponent));
		if (!characterDamageManager)
			return;

		characterDamageManager.AddParticularBleeding();
	}
};