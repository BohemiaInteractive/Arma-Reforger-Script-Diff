[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AIWaypointArtillerySupportAmmoTypeAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableWaypointComponent editableWpComp = SCR_EditableWaypointComponent.Cast(item);
		if (!editableWpComp)
			return null;
		SCR_AIWaypointArtillerySupport wp = SCR_AIWaypointArtillerySupport.Cast(editableWpComp.GetOwner());
		if (!wp)
			return null;
		
		SCR_EAIArtilleryAmmoType ammoType = wp.GetAmmoType();
		int index = ConvertValueToIndex(ammoType);
		
		if (index == -1)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(index);
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SCR_EditableWaypointComponent editableWpComp = SCR_EditableWaypointComponent.Cast(item);
		if (!editableWpComp)
			return;
		SCR_AIWaypointArtillerySupport wp = SCR_AIWaypointArtillerySupport.Cast(editableWpComp.GetOwner());
		if (!wp)
			return;
			
		int index = var.GetInt();
		float rawValue;
		if (!ConvertIndexToValue(index, rawValue))
			return;
		
		SCR_EAIArtilleryAmmoType ammoType = rawValue;
		wp.SetAmmoType(ammoType);
	}
}