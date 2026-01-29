[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_SelectedEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	protected ref map<int, ref set<SCR_EditableEntityComponent>> m_SavedSelections = new map<int, ref set<SCR_EditableEntityComponent>>();
	
	/*!
	Save currently selected entities.
	\param Selection index
	*/
	void SaveSelection(int index)
	{
		set<SCR_EditableEntityComponent> selection = new set<SCR_EditableEntityComponent>();
		selection.Copy(m_Entities);
		m_SavedSelections.Set(index, selection);
	}
	/*!
	Load currently selected entities.
	\param Selection index
	*/
	void LoadSelection(int index)
	{
		set<SCR_EditableEntityComponent> selection = new set<SCR_EditableEntityComponent>();
		if (GetSelection(index, selection))
		{
			Replace(selection);
		}
	}
	/*!
	Load currently selected entities and teleport to the first entity in the list.
	\param Selection index
	*/
	void TeleportSelection(int index)
	{
		SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
		if (!camera)
			return;
		
		set<SCR_EditableEntityComponent> selection = new set<SCR_EditableEntityComponent>();
		if (GetSelection(index, selection))
		{
			SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
			if (teleportComponent)
			{
				vector position;
				if (selection[0].GetPos(position))
				{
					teleportComponent.TeleportCamera(position);
					//Replace(selection); //--- Not needed, single-click of the key already selects the entity
				}
			}
		}
	}
	protected bool GetSelection(int index, out notnull set<SCR_EditableEntityComponent> outSelection)
	{
		if (!m_SavedSelections.Find(index, outSelection))
			return false;
		
		for (int i = outSelection.Count() - 1; i >= 0; i--)
		{
			if (!outSelection[i])
				outSelection.Remove(i);
		}
		return !outSelection.IsEmpty();
	}
	
	protected void OnParentEntityChanged(SCR_EditableEntityComponent entity, SCR_EditableEntityComponent parentEntity, SCR_EditableEntityComponent parentEntityPrev)
	{
		//--- Select if the parent is selected as well
		if (parentEntity && parentEntity.HasEntityState(EEditableEntityState.SELECTED))
			Add(entity);
	}
	
	override void EOnEditorActivate()
	{
		if (!GetCore()) return;
		GetCore().Event_OnParentEntityChanged.Insert(OnParentEntityChanged);
	}
	override void EOnEditorDeactivate()
	{
		if (!GetCore()) return;
		GetCore().Event_OnParentEntityChanged.Remove(OnParentEntityChanged);
	}
};