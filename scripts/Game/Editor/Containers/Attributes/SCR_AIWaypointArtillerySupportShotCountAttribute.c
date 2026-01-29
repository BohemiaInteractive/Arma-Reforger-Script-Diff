[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AIWaypointArtillerySupportShotCountAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	/*
	This attribute encodes two states of waypoint: 1. Shot count 2. Active.
	
	When WP is not active, bots wait for us to provide fire mission values.
	If shot count value at waypoint is -1, bots fire infinitely.
	
	Therefore:
	Index 0 -> WP is not active, shot count is 0
	Index 1...N-2 -> WP is active, shot count is the value stored at given index
	Last index -> WP is active, shot count is -1, meaning infinite
	*/
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableWaypointComponent editableWpComp = SCR_EditableWaypointComponent.Cast(item);
		if (!editableWpComp)
			return null;
		SCR_AIWaypointArtillerySupport wp = SCR_AIWaypointArtillerySupport.Cast(editableWpComp.GetOwner());
		if (!wp)
			return null;
		
		bool active = wp.IsActive();
		int count = wp.GetTargetShotCount();
		
		int index = 0; // If it's not active, then index is 0
		if (active)
		{
			if (count == -1) // -1 value on waypoint -> infinite
				index = m_aValues.Count()-1; // Last index
			else
				index = ConvertValueToIndex(count);
		}
		
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
		
		bool active = index != 0;
		int count;
		if (index == (m_aValues.Count()-1)) // Last index
			count = -1; // -1 value on waypoint -> infinite
		else
			count = rawValue;
		
		wp.SetTargetShotCount(count, false);
		wp.SetActive(active, true);
	}
}