//! @ingroup Editor_UI Editor_UI_Components

class SCR_EditableEntityLinkUIComponent : MenuRootSubComponent
{
	protected SCR_EditableEntityComponent m_LinkedEntity;
	protected SCR_HoverEditableEntityFilter m_HoverManager;
	
	//------------------------------------------------------------------------------------------------
	//! Set the entity this widget represents.
	//! \param[in] entity Editable entity
	void SetLinkedEntity(SCR_EditableEntityComponent entity)
	{
		m_LinkedEntity = entity;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the entity this widget represents.
	//! \return Editable entity
	SCR_EditableEntityComponent GetLinkedEntity()
	{
		return m_LinkedEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate()
	{
		if (!m_HoverManager || !m_LinkedEntity)
			return;
		
		m_HoverManager.SetEntityUnderCursor(m_LinkedEntity, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (!m_LinkedEntity)
			return false;
		
		MenuRootBase menu = GetMenu();
		if (!menu)
			return false;
		
		menu.GetOnMenuUpdate().Insert(OnMenuUpdate);		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		MenuRootBase menu = GetMenu();
		if (!menu)
			return false;
		
		menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsUnique()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		m_HoverManager = SCR_HoverEditableEntityFilter.Cast(SCR_HoverEditableEntityFilter.GetInstance(EEditableEntityState.HOVER, true));
	}
}
