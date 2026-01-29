[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_FactionGroupsTooltipDetail : SCR_EntityTooltipDetail
{
	protected TextWidget m_Text;
	protected AIWorld m_AIWorld;
	protected SCR_BaseEditableEntityFilter m_Filter;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		int count = 0;
		Faction faction = entity.GetFaction();
		
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		m_Filter.GetEntities(entities);
		foreach (SCR_EditableEntityComponent ent: entities)
		{
			if (ent.GetEntityType() == EEditableEntityType.GROUP && ent.GetFaction() == faction)
				count++;
		}

		m_Text.SetText(count.ToString());
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Filter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.VISIBLE, true);
		if (!m_Filter)
			return false;
		
		m_Text = TextWidget.Cast(widget);
		return m_Text != null;
	}
}
