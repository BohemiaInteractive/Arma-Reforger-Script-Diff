[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_FactionPlayersTooltipDetail : SCR_EntityTooltipDetail
{
	protected TextWidget m_Text;
	protected SCR_FactionManager m_FactionManager;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		Faction faction = entity.GetFaction();
		m_Text.SetText(m_FactionManager.GetFactionPlayerCount(faction).ToString());
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!m_FactionManager)
			return false;
		
		m_Text = TextWidget.Cast(widget);
		return m_Text != null;
	}
}
