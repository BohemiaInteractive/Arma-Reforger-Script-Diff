//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
/** @ingroup Editor_Context_Actions
*/
/*!
Base container class for general editor context actions, not related to a hovered/selected entity
*/
class SCR_GeneralContextAction : SCR_BaseContextAction
{
	[Attribute(desc: "Should this action be hidden when an entity is hovered")]
	protected bool m_HideOnHover;
	[Attribute(desc: "Should this action be hidden when any entities are selected")]
	protected bool m_HideOnSelected;
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if(m_HideOnHover && hoveredEntity)
		{
			return false;
		}
		if(m_HideOnSelected && !selectedEntities.IsEmpty())
		{
			return false;
		}
		return true;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		PrintFormat("This is general action %1, description: %2", GetInfo().GetName(), GetInfo().GetDescription());
	}
};
