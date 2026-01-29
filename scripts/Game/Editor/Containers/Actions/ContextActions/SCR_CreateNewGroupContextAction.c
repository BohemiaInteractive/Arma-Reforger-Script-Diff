// Script File
//Get all characters. Check if A group (or more) is selected. Check if all characters in the group are selected. Ignore if true. If only of the same group do not preform

[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_CreateNewGroupContextAction : SCR_BaseContextAction
{	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (hoveredEntity && (!selectedEntities.Contains(hoveredEntity) || SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(hoveredEntity.GetOwner()) > 0))
			return false;
		
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			if (entity.GetEntityType() == EEditableEntityType.CHARACTER && SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(entity.GetOwner()) <= 0)
				return true;
		}
		
		return false;
	}
	
	//Can always preform if has AI characters
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{				
		SCR_EditableEntityComponent parentGroup = null;
		int entityCount = 0;
		
		//!Make sure that if all selected characters are in the same group and the whole group is selected that the player cannot create a new group (as this would change nothing)
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			if (entity.GetEntityType() == EEditableEntityType.CHARACTER && SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(entity.GetOwner()) <= 0)
			{
				SCR_EditableEntityComponent newParentGroup = entity.GetParentEntity();
				
				entityCount++;
				
				if (parentGroup == null)
					parentGroup = newParentGroup;
				else if (parentGroup != newParentGroup)
					return true;
			}
		}
		
		return (parentGroup && parentGroup.GetChildrenCount() != entityCount);
	}
	
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{			
		SCR_EditableEntityComponent group;
		SCR_EditableCharacterComponent newLeader;
		
		foreach (SCR_EditableEntityComponent entity: selectedEntities)
		{
			if (entity.GetEntityType() != EEditableEntityType.CHARACTER || SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(entity.GetOwner()) > 0)
				continue;
			
			if (!group)
			{
				newLeader = SCR_EditableCharacterComponent.Cast(entity);
				
				if (newLeader)
					group = newLeader.CreateGroupForCharacter();
				
				continue;
			}
			
			entity.SetParentEntity(group);
		}

	}

	
};