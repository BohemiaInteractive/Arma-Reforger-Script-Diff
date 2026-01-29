[BaseContainerProps()]
class SCR_EditableGroupInteraction: SCR_EditableEntityInteraction
{
	override bool CanSetParent(EEditableEntityType parentType, EEditableEntityFlag parentFlags, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		if (parentType == ROOT)
			return true;
		
		return parentType == EEditableEntityType.CHARACTER
			|| parentType == EEditableEntityType.GROUP
			|| parentType == EEditableEntityType.VEHICLE;   
	}
};