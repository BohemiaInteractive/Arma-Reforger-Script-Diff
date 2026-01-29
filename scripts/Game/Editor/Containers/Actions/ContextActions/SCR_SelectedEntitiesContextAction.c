//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
/** @ingroup Editor_Context_Actions
*/
/*!
Base container class for editor context actions which apply to selected entities

See SCR_DeleteSelectedContextAction
*/
class SCR_SelectedEntitiesContextAction : SCR_BaseContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			if (CanBeShown(entity, cursorWorldPosition, flags))
			{
				return true;
			}
		}		
		return false;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			if (CanBePerformed(entity, cursorWorldPosition, flags))
			{
				return true;
			}
		}
		return false;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (!InitPerform()) return;
		
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			if (CanBePerformed(entity, cursorWorldPosition, 0))
			{
				Perform(entity, cursorWorldPosition);
			}
		}
	}
	
	override void PerformOwner(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (!InitPerform()) return;
		
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			if (CanBePerformed(entity, cursorWorldPosition, 0))
			{
				PerformOwner(entity, cursorWorldPosition);
			}
		}
	}
	
	/*!
	Helper function for actions that should be performed on multiple entities, avoids having foreach loops in each subclass
	\param selectedEntity Current selectedEntities iteration
	*/
	bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	/*!
	Helper function to execute code before performing the action on all entities, called only once (eg. clear selection)
	*/
	bool InitPerform()
	{
		return true;
	}
	
	/*!
	Perform function for each selected entity
	\param selectedEntity Current selectedEntities iteration
	\param cursorWorldPosition Cursor world position
	*/
	void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		
	}
	
	/*!
	Perform function for each selected entity, executed on owner, player that executed the action
	\param selectedEntity Current selectedEntities iteration
	\param cursorWorldPosition Cursor world position
	*/
	void PerformOwner(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		
	}
};