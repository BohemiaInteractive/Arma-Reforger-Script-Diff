[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_FactionHintTooltipDetail : SCR_EntityTooltipDetail
{
	[Attribute("Text")]
	protected string m_sTextWidgetName;

	[Attribute("Seperator")]
	protected string m_sSeperatorWidgetName;

	[Attribute("160")]
	protected int m_iSeperatorAlpha;
	
	[Attribute("#AR-Editor_TooltipDetail_FactionTasks_Hint")]
	protected LocalizedString m_TaskHintText;

	[Attribute("#AR-Editor_TooltipDetail_FactionSpawnPoint_Hint")]
	protected LocalizedString m_SpawnPointHintText;
	
	protected RichTextWidget m_Text;
	protected ImageWidget m_Seperator;
	protected Widget m_self;
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{		
		m_self = widget;
		
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem) 
			return false;
			
		m_Text = RichTextWidget.Cast(widget.FindAnyWidget(m_sTextWidgetName));
		if (!m_Text)
			return false;
		
		m_Text.SetText(string.Empty);
		
		SCR_DelegateFactionManagerComponent m_DelegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (!m_DelegateFactionManager) 
			return false;
		
		Faction faction = entity.GetFaction();
		if (!faction)
			return false;
		
		SCR_EditableFactionComponent editableFaction = m_DelegateFactionManager.GetFactionDelegate(faction);
		if (!editableFaction)
			return false;
		
		int spawnPointCount = editableFaction.GetFactionSpawnPointCount();
		int taskCount = editableFaction.GetFactionTasksCount();
		
		m_Seperator = ImageWidget.Cast(widget.FindAnyWidget(m_sSeperatorWidgetName));
		if (m_Seperator)
		{
			Color factionColor = faction.GetFactionColor();
			m_Seperator.SetColor(new Color(factionColor.R(), factionColor.G(), factionColor.B(), m_iSeperatorAlpha));
		}
		
		if (taskCount == 0)
			m_Text.SetText(m_TaskHintText);

		if (spawnPointCount == 0)
		{
			if (taskCount == 0)
				m_Text.SetText(m_Text.GetText() + "<br/><br/>");
			
			m_Text.SetText(m_Text.GetText() + m_SpawnPointHintText);
		}
		
		return taskCount == 0 || spawnPointCount == 0;
	}
}
