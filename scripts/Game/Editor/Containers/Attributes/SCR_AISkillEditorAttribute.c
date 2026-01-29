[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AiSkillEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{	
	//---- REFACTOR NOTE START: Not up to date with internal scripting style ----
	// Probably not implemeneted
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity || (editableEntity.GetEntityType() != EEditableEntityType.CHARACTER && editableEntity.GetEntityType() != EEditableEntityType.GROUP)) return null;
		if (editableEntity.HasEntityState(EEditableEntityState.PLAYER)) return null;
		
		//WIP
		#ifndef WORKBENCH
		return null;
		#endif
		
		int rank;
		
		//If character
		if (editableEntity.GetEntityType() == EEditableEntityType.CHARACTER)
		{
			rank = SCR_CharacterRankComponent.GetCharacterRank(editableEntity.GetOwner());
		}
		//If group
		else
		{
			SCR_AIGroup aiGroup = SCR_AIGroup.Cast(editableEntity.GetOwner()); 
			if (!aiGroup) return null;
			rank = SCR_CharacterRankComponent.GetCharacterRank(aiGroup.GetLeaderEntity());
		}
		
		return SCR_BaseEditorAttributeVar.CreateInt(rank);
	}	
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) return;
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		
		if (editableEntity.GetEntityType() == EEditableEntityType.CHARACTER)
		{
			Print("Set Character skill to: " +  m_aValues[var.GetInt()].GetName());
		}
		else if (editableEntity.GetEntityType() == EEditableEntityType.GROUP)
		{
			Print("Set Group skill to: " +  m_aValues[var.GetInt()].GetName());
		}
		
		//AI rank not in game
		
		/*
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity.GetEntityType() == EEditableEntityType.CHARACTER && !editableEntity.GetEntityType() == EEditableEntityType.GROUP) return;
		if (editableEntity.HasEntityState(EEditableEntityState.PLAYER)) return;
		
		int rank = var.GetInt();
		
		//If character
		if (editableEntity.GetEntityType() == EEditableEntityType.CHARACTER)
		{
			SCR_CharacterRankComponent rankComponent = SCR_CharacterRankComponent.Cast(editableEntity.FindComponent(SCR_CharacterRankComponent));
			if (!rankComponent) return;
			
			rankComponent.SetCharacterRank(rank);
			
			rank = SCR_CharacterRankComponent.GetCharacterRank(editableEntity.GetOwner());
		}
		//If group
		else
		{
			Print("Set ranks of group members");
			
			//Set group rank
			AIGroup group = SCR_EditableGroupComponent.Cast(editableEntity).GetGroup();
			
			array<AIAgent> agents;
			group.GetAgents(agents);
			int count = agents.Count();
			
			for (int i = 0; i < count; ++i)
			{
				
			}
		}*/
	}
	//---- REFACTOR NOTE END ----
};
