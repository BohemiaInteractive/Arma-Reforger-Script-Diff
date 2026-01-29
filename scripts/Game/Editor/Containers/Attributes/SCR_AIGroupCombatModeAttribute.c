[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AIGroupCombatModeAttribute: SCR_BaseFloatValueHolderEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableGroupComponent editableGroupComponent = SCR_EditableGroupComponent.Cast(item);
		
		if (!editableGroupComponent)
			return null;
		
		SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(editableGroupComponent.GetOwner().FindComponent(SCR_AIGroupUtilityComponent));
		if (!utility)
			return null;
		
		if (!ValidateEnumValues(EAIGroupCombatMode, true))
			return null;
		
		EAIGroupCombatMode combatMode = utility.GetCombatModeExternal();
		
		int index = ConvertValueToIndex(combatMode);
		if (index != -1)
			return SCR_BaseEditorAttributeVar.CreateInt(index);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SCR_EditableGroupComponent editableGroupComponent = SCR_EditableGroupComponent.Cast(item);
		
		if (!editableGroupComponent)
			return;
		
		SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(editableGroupComponent.GetOwner().FindComponent(SCR_AIGroupUtilityComponent));
		if (!utility)
			return;
		
		int index = var.GetInt();
		
		float combatModeFloat;
		if (!ConvertIndexToValue(index, combatModeFloat))
			return;
		
		EAIGroupCombatMode combatMode = combatModeFloat;
		
		utility.SetCombatMode(combatMode);
	}
}