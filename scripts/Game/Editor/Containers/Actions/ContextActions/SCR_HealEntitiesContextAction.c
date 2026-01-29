//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_HealEntitiesContextAction : SCR_SelectedEntitiesContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{		
		if (selectedEntity.GetEntityType() == EEditableEntityType.VEHICLE || selectedEntity.GetEntityType() == EEditableEntityType.CHARACTER)
		{
			return true;
		}
		return false;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		SCR_DamageManagerComponent damageComponent = SCR_DamageManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(SCR_DamageManagerComponent));
		if (!damageComponent)
			return false;
		
		if (damageComponent.GetState() == EDamageState.DESTROYED)
			return false;

		return damageComponent.CanBeHealed();
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		SCR_DamageManagerComponent damageComponent = SCR_DamageManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(SCR_DamageManagerComponent));
		if (!damageComponent)
			return;
		
		damageComponent.FullHeal();
	}
};
