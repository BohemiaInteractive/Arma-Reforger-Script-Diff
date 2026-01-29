[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_MapGridTooltipDetail : SCR_EntityTooltipDetail
{
	protected TextWidget m_Text;
	protected SCR_MapEntity m_Map;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		vector pos;
		if (entity.GetPos(pos))
			m_Text.SetText(m_Map.GetGridLabel(pos));
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Text = TextWidget.Cast(widget);
		m_Map = SCR_MapEntity.GetMapInstance();
		return m_Text && m_Map;
	}
}
