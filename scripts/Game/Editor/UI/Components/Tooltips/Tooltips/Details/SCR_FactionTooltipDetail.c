[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_FactionTooltipDetail : SCR_EntityTooltipDetail
{
	[Attribute("#AR-Editor_TooltipDetail_FactionUnassigned_Name")]
	protected LocalizedString m_sPlayerNoFactionName;
	
	protected TextWidget m_Text;
	protected SCR_FactionManager m_FactionManager;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		Faction faction = entity.GetFaction();
		
		if (!faction && m_FactionManager)
		{
			faction = m_FactionManager.GetPlayerFaction(entity.GetPlayerID());
			
			//Player has no faction assigned yet
			if (!faction)
			{
				m_Text.SetText(m_sPlayerNoFactionName);
				return;
			}
		}
			
		if (faction) 
			m_Text.SetText(faction.GetFactionName());
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Text = TextWidget.Cast(widget);
		
		Faction faction = entity.GetFaction();
		int playerID = -1;
		
		if (!faction)
		{
			playerID = entity.GetPlayerID();
			m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		}
		
		return m_Text && (faction || (playerID > 0 && m_FactionManager));
	}
}
