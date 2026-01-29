[BaseContainerProps()]
class SCR_EditableAttachableEntityInteraction : SCR_EditableEntityInteraction
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	protected ref array<EEditableEntityType> m_aAttachableTypes;

	//------------------------------------------------------------------------------------------------
	override bool CanSetParent(SCR_EditableEntityComponent parentEntity, EEditableEntityType parentType, EEditableEntityFlag parentFlags, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		//++ Check if it has a already the maximum attached entities
		if (parentEntity && parentEntity.GetAttachedEntities().Count() >= EditorConstants.MAX_ATTACHED_ENTITIES)
			return false;
		
		//++ Allow it to be freely placed on ground level
		if (parentType == ROOT)
			return true;

		//++ If is not ground level, check if it is allowed (using whitelist)
		return m_aAttachableTypes.Contains(parentType);
	}
	
	void SCR_EditableAttachableEntityInteraction()
	{
		m_bCanRefParent = true;
	}
}

class EditorConstants
{
	static const int MAX_ATTACHED_ENTITIES = 5;
}