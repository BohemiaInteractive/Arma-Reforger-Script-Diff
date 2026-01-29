class SCR_CurrentTaskUIComponent : SCR_ScriptedWidgetComponent
{	
	[Attribute("#AR-Tasks_NoAssignedTaskTitle")]
	protected string m_sNoAssignedTaskTitle;
	
	[Attribute("#AR-Tasks_NoAssignedTaskDescription")]
	protected string m_sNoAssignedTaskDescription;
	
	protected ref SCR_CurrentTaskWidgets m_Widgets = new SCR_CurrentTaskWidgets();
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(w);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateTask(SCR_Task task)
	{	
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		if (!faction)
			return;

		if (!task)
		{	
			Color color = faction.GetOutlineFactionColor();
			if (color)
			{	
				m_Widgets.m_wTaskIcon.SetColor(color);
				m_Widgets.m_wTaskIconOutline.SetColor(color);
			}
			
			m_Widgets.m_wTaskTitle.SetText(m_sNoAssignedTaskTitle);
			m_Widgets.m_wTaskDescription.SetText(m_sNoAssignedTaskDescription);
			
			return;
		}
		
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		// Handle progression bar
		if (taskSystem.CanProgressBeShownForTask(task))
		{
			float progress = taskSystem.GetTaskProgress(task);
			m_Widgets.m_wProgressBar.SetVisible(true);
			m_Widgets.m_ProgressBarComponent.SetValue(progress);
		}
		
		SCR_TaskUIInfo info = task.GetTaskUIInfo();
		if (!info)
			return;

		info.SetNameTo(m_Widgets.m_wTaskTitle);
		info.SetDescriptionTo(m_Widgets.m_wTaskDescription);
		info.SetIconTo(m_Widgets.m_wTaskIconSymbol);
		
		Color color = faction.GetOutlineFactionColor();
		if (color)
		{	
			m_Widgets.m_wTaskIcon.SetColor(color);
			m_Widgets.m_wTaskIconOutline.SetColor(color);
		}
		
		SCR_TaskExecutor executor = SCR_TaskExecutor.FromPlayerID(playerID);
		if (!executor)
			return;
		
		// Check if current controlled Entity is assigned to the task
		if (task.IsTaskAssignedTo(executor))
		{
			Color factionColor = faction.GetFactionColor();
			if (factionColor)
				m_Widgets.m_wTaskIconBackground.SetColor(factionColor);
		}
	}
}
