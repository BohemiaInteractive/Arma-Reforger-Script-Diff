[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_FactionTasksAndSpawnPointsTooltipDetail : SCR_TextAndWarningTooltipDetail
{	
	protected SCR_DelegateFactionManagerComponent m_DelegateFactionManager;
	
	[Attribute(desc: "If true shows Tasks, If false Shows Spawnpoint")]
	protected bool m_bShowTasksTooltip;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		if (!m_DelegateFactionManager) 
			return; 
		
		Faction faction = entity.GetFaction();
		if (!faction)
			return;
		
		SCR_EditableFactionComponent editableFaction = m_DelegateFactionManager.GetFactionDelegate(faction);
		if (!editableFaction)
			return;
		
		int count;
		
		if (m_bShowTasksTooltip)
			count = editableFaction.GetFactionTasksCount();
		else
			count = editableFaction.GetFactionSpawnPointCount();

		m_Text.SetText(count.ToString());
		
		//Warning Text
		ToggleWarning(count == 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{		
		m_DelegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (!m_DelegateFactionManager)
			return false;
		
		return super.InitDetail(entity, widget);
	}
}
