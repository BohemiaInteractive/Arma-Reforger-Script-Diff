class SCR_EntityToolbarItemEditorUIComponent : SCR_BaseToolbarItemEditorUIComponent
{
	[Attribute()]
	protected string m_sIconWidgetName;
	
	[Attribute()]
	protected string m_sNameWidgetName;
	
	protected SCR_EditableEntityComponent m_Entity;
	protected TextWidget m_NameWidget;
	
	//------------------------------------------------------------------------------------------------
	override void InitToolbarItem(Widget widget)
	{	
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \param[in] widget
	//! \param[in] slotManager
	void SetEntity(SCR_EditableEntityComponent entity, Widget widget, SCR_EditableEntitySlotManagerUIComponent slotManager)
	{
		if (slotManager)
		{		
			Widget slotWidget = widget.FindAnyWidget(m_sIconWidgetName);
			if (slotWidget)
			{
				SCR_EditableEntityBaseSlotUIComponent slot = SCR_EditableEntityBaseSlotUIComponent.Cast(slotWidget.FindHandler(SCR_EditableEntityBaseSlotUIComponent));
				if (slot)
					slotManager.InsertSlotExternal(entity, slot);
			}
		}
		else
		{
			Debug.Error2(ToString(), string.Format("SCR_EditableEntitySlotManagerUIComponent UI component is missing on %1!", widget.GetName()));
		}
		
		SCR_UIInfo info = entity.GetInfo();
		if (info)
		{
			m_NameWidget = TextWidget.Cast(widget.FindAnyWidget(m_sNameWidgetName));
			if (m_NameWidget)
			{
				//--- Set name after a delay - GetPlayerID() would return incorrect value for newly created players
				GetGame().GetCallqueue().CallLater(SetName, 1, false, entity);
			
				//--- Add refresh event (e.g., when group identity changes)
				if (entity.GetOnUIRefresh())
				{
					entity.GetOnUIRefresh().Insert(RefreshName);
					m_Entity = entity;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetName(SCR_EditableEntityComponent entity)
	{
		int playerID = entity.GetPlayerID();
		if (playerID > 0)
		{
			m_NameWidget.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID));
		}
		else
		{
			SCR_UIInfo info = entity.GetInfo();
			if (info)
				info.SetNameTo(m_NameWidget);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshName()
	{
		SetName(m_Entity);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_Entity)
			m_Entity.GetOnUIRefresh().Remove(RefreshName);
	}
}
