[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_WaypointIndexTooltipDetail : SCR_EntityTooltipDetail
{
	protected TextWidget m_Text;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		int index;
		SCR_EditableWaypointComponent waypoint = SCR_EditableWaypointComponent.Cast(entity);
		if (waypoint)
			index = waypoint.GetWaypointIndex();
		
		m_Text.SetText(index.ToString());
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Text = TextWidget.Cast(widget);
		return m_Text != null && entity.GetEntityType() == EEditableEntityType.WAYPOINT;
	}
}
