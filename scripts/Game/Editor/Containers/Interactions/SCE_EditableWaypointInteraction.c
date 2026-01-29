[BaseContainerProps()]
class SCR_EditableWaypointInteraction: SCR_EditableEntityInteraction
{
	override bool CanSetParent(EEditableEntityType parentType, EEditableEntityFlag parentFlags, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		if (parentType == ROOT)
			return SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.PLACING);
		
		return !SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.PLACING) //--- Don't allow any interaction while placing, it would add the waypoint to the target instead
			&&
			(
				parentType == EEditableEntityType.CHARACTER
				|| parentType == EEditableEntityType.GROUP
			);
	}
};