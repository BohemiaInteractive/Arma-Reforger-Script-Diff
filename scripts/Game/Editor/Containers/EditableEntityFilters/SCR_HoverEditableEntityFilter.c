[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_HoverEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	[Attribute(desc: "State required for the entity to become hovered on.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	private EEditableEntityState m_RequiredState;
	
	private SCR_EditableEntityComponent m_EntityUnderCursorCandidate;
	private bool m_bIsEntityUnderCursorDelegate;
	private bool m_bIsEntityUnderCursorDelegateCandidate;
	private bool m_EntityUnderCursorEnabled = true;
	private SCR_BaseEditableEntityFilter m_RequiredFilter;
	
	protected void UpdateEntityUnderCursor()
	{
		if (!m_EntityUnderCursorEnabled) return;
		
		if (m_EntityUnderCursorCandidate != GetFirstEntity())
		{
			//--- Update predecessor state (e.g, when FOCUSED is used)
			if (m_RequiredFilter)
			{
				if (m_EntityUnderCursorCandidate)
					m_RequiredFilter.Replace(GetParentBelowCurrentLayer(m_EntityUnderCursorCandidate));
				else
					m_RequiredFilter.Clear();
			}
			
			m_bIsEntityUnderCursorDelegate = m_bIsEntityUnderCursorDelegateCandidate; //--- Make sure the variable is available in OnChange() events
			Replace(m_EntityUnderCursorCandidate, true);
		}
		else
		{
			bool delegateChanged = m_bIsEntityUnderCursorDelegate != m_bIsEntityUnderCursorDelegateCandidate;
			m_bIsEntityUnderCursorDelegate = m_bIsEntityUnderCursorDelegateCandidate;
			if (delegateChanged) OnChanged();
		}
		m_EntityUnderCursorCandidate = null;
	}
	
	/*!
	Set which entity is under cursor.
	\param entity Editable entity
	\param isDelegate True if the entity under cursor is represented by a delegate (e.g., icon), not a geometry. Delegates have higher priority, for example for camera snapping.
	*/
	void SetEntityUnderCursor(SCR_EditableEntityComponent entity, bool isDelegate = false)
	{
		if (!entity && m_EntityUnderCursorCandidate) return; //--- Cannot null existing candidate
		m_EntityUnderCursorCandidate = entity;
		m_bIsEntityUnderCursorDelegateCandidate = isDelegate;
		//if (entity) entity.Log(isDelegate.ToString(), true);
	}
	/*!
	Get entity under cursor.
	\param[out] isDelegate True if the entity under cursor is represented by a delegate (e.g., icon)
	\return Editable entity
	*/
	SCR_EditableEntityComponent GetEntityUnderCursor(out bool isDelegate = false)
	{
		isDelegate = m_bIsEntityUnderCursorDelegate;
		return GetFirstEntity();
	}
	/*!
	Get interactive entity under cursor (e.g., one which can be selected)
	\return Editable entity
	*/
	SCR_EditableEntityComponent GetInteractiveEntityUnderCursor()
	{
		SCR_EditableEntityComponent entity = GetFirstEntity();
		if (!entity || entity.HasEntityFlag(EEditableEntityFlag.NON_INTERACTIVE)) return null;
		return entity;
	}
	/*!
	Get candidate entity under cursor.
	Multiple systems can set entity under cursor each frame, and the final one is assigned at the end of the frame.
	This is useful for checking of a candidate already exists, so low priority candidate would be ignored.
	\param[out] isDelegate True if the entity under cursor candidate is represented by a delegate (e.g., icon)
	\return Editable entity
	*/
	SCR_EditableEntityComponent GetEntityUnderCursorCandidate(out bool isDelegate = false)
	{
		isDelegate = m_bIsEntityUnderCursorDelegateCandidate;
		return m_EntityUnderCursorCandidate;
	}
	/*!
	Set if the entity under cursor can be assigned.
	Important for systems that could collide with entity under cursor (e.g., selection frame).
	\param enable True to enable asisgning (default: true)
	*/
	void SetEntityUnderCursorEnabled(bool enabled)
	{
		if (enabled == m_EntityUnderCursorEnabled) return;
		
		if (enabled)
		{
			//--- Refresh required filter in case some other system changed it
			if (m_RequiredFilter)
			{
				if (GetFirstEntity())
					m_RequiredFilter.Replace(GetParentBelowCurrentLayer(GetFirstEntity()));
				else
					m_RequiredFilter.Clear();
			}
		}
		else
		{
			//--- Cancel the current entity under cursor when setting is disabled
			SetEntityUnderCursor(null);
			UpdateEntityUnderCursor();
		}
		m_EntityUnderCursorEnabled = enabled;
	}
	/*!
	Get if the entity under cursor can be assigned.
	\return True to enable asisgning (default: true)
	*/
	bool GetEntityUnderCursorEnabled()
	{
		return m_EntityUnderCursorEnabled;
	}
	/*!
	Check if the entity under cursor is represented by a delegate (e.g., icon)
	\return True when delegate
	*/
	bool IsDelegate()
	{
		return m_bIsEntityUnderCursorDelegate;
	}
	
	override void EOnEditorFrame(float timeSlice)
	{
		UpdateEntityUnderCursor();
	}
	override void EOnEditorActivate()
	{
		m_RequiredFilter = GetManager().GetFilter(m_RequiredState);
	}
};