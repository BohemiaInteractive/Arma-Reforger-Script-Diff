[BaseContainerProps()]
class SCR_EditableCharacterInteraction: SCR_EditableEntityInteraction
{
	override bool CanSetParent(EEditableEntityType parentType, EEditableEntityFlag parentFlags, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		//--- Killed characters can be only in root
		if (!SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.ALIVE))
			return parentType == ROOT;
		
		if (parentType == ROOT)
			return true;
		
		//--- Cannot move players to groups
		if (parentType != EEditableEntityType.VEHICLE && !SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.NON_PLAYABLE))
			return false;
	
		//--- AI-compatible target
		return parentType == EEditableEntityType.CHARACTER
			|| parentType == EEditableEntityType.GROUP
			|| parentType == EEditableEntityType.VEHICLE; 
	}
};