[BaseContainerProps()]
class SCR_EditableEntityInteraction
{
	protected bool m_bCanRefParent; //++ This can be set in inherited classes' constructor
	
	static const int ROOT = -1;
	
	/*!
	Check if the entity can be moved to intended parent.
	\param parentEntity New parent. Null when evaluating root.
	\params interactionFlags Flags defining details about the interaction (all are enabled if undefined)
	\return True if interaction is possible
	*/
	sealed bool CanSetParent(SCR_EditableEntityComponent parentEntity, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		if (parentEntity)
		{
			if (m_bCanRefParent)
				return CanSetParent(parentEntity, parentEntity.GetEntityType(), parentEntity.GetEntityFlags(), interactionFlags);
			
			return CanSetParent(parentEntity.GetEntityType(), parentEntity.GetEntityFlags(), interactionFlags);
		}
		else
		{
			if (m_bCanRefParent)
				return CanSetParent(null, ROOT, 0, interactionFlags);
			
			return CanSetParent(ROOT, 0, interactionFlags);
		}
	}
	/*!
	Check if the entity can be moved to a parent with given params.
	\param parentType Type of the new parent, use SCR_EditableEntityInteraction.ROOT if it§s root of editable entities.
	\param parentFlags Flags of the new parent
	\params interactionFlags Flags defining details about the interaction (all are enabled if undefined)
	\return True if interaction is possible
	*/
	bool CanSetParent(EEditableEntityType parentType, EEditableEntityFlag parentFlags, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		if (!SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.DELEGATE) || !SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.LAYER_EDITING))
			return false;
		
		if (parentType == ROOT)
			return true;
		
		if (!SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.DELEGATE))
			return false;
		
		if ((parentFlags & EEditableEntityFlag.LAYER) != EEditableEntityFlag.LAYER)
			return false;
		
		return parentType == EEditableEntityType.GENERIC;
	}
	/*!
	Check if the entity can be moved to a parent with given params. Only called if m_bCanRefParent is set to true
	\param parentEntity New parent. Null when evaluating root.
	\param parentType Type of the new parent, use SCR_EditableEntityInteraction.ROOT if it§s root of editable entities.
	\param parentFlags Flags of the new parent
	\params interactionFlags Flags defining details about the interaction (all are enabled if undefined)
	\return True if interaction is possible
	*/
	bool CanSetParent(SCR_EditableEntityComponent parentEntity, EEditableEntityType parentType, EEditableEntityFlag parentFlags, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		//++ Do something with parent
		return CanSetParent(parentType, parentFlags, interactionFlags);
	}
	
	/*!
	Check if new layer can be created for entity
	\param EEditableEntityType new layer type
	\param newLayerFlags new layer m_Flags
	\param parentEntity parent entity
	\param CheckParentEntity if should check if new layer can be added to parent
	\return bool if the given layer can be created for the entity
	*/
	bool CanCreateParentFor(EEditableEntityType newLayerType, EEditableEntityFlag newLayerFlags, SCR_EditableEntityComponent parentEntity, bool CheckParentEntity = true)
	{
		if (CheckParentEntity && parentEntity)
		{
			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			
			if (core)
			{
				SCR_EditableEntityInteraction interaction = core.GetEntityInteraction(EEditableEntityType.GENERIC);
				if (!interaction.CanSetParent(parentEntity))
					return false;
			}
		}
		
		return CanSetParent(newLayerType, newLayerFlags);
	}
};