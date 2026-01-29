class SCR_GameOverScreenCombatControlPatrolUIComponent: SCR_GameOverScreenContentUIComponent
{
	[Attribute("GameOver_Time")]
	protected string m_sTime;
	
	[Attribute("GameOver_Tasks")]
	protected string m_sTasks;
	
	protected string 					timeElapsed;
	
	override void InitContent(SCR_GameOverScreenUIContentData endScreenUIContent)
	{
		super.InitContent(endScreenUIContent);
		
		RichTextWidget titleWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sTileName));
		RichTextWidget subtitleWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sSubtitleName));
		RichTextWidget timeWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sTime));
		RichTextWidget tasksWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sTasks));
		
		if (titleWidget)
			titleWidget.SetTextFormat(endScreenUIContent.m_sTitle, endScreenUIContent.m_sTitleParam);

		if (subtitleWidget)
			subtitleWidget.SetTextFormat(endScreenUIContent.m_sSubtitle, endScreenUIContent.m_sSubtitleParam);
		
		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!baseGameMode)
			return;
		
		int days, hours, minutes, seconds;
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(baseGameMode.GetElapsedTime(), days, hours, minutes, seconds);
		
		timeElapsed = string.Format(WidgetManager.Translate("#AR-CareerProfile_TimePlayed_TotalValue", ""+days, ""+hours, ""+minutes, ""+seconds));
		
		if (timeWidget)
			timeWidget.SetTextFormat(string.Format("<color rgba=%1>%2</color> <br/> %3", UIColors.FormatColor(UIColors.CONTRAST_COLOR), "#AR-CombatScenario_Total_Operation_Time",  timeElapsed));
		
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		array<SCR_Task> allTasks = {};
		taskSystem.GetTasksByState(allTasks, SCR_ETaskState.COMPLETED);
		
		string tasksToShow = "<br/>";
		SCR_TaskUIInfo taskUiInfo;
		
		foreach (SCR_Task task : allTasks)
		{
			taskUiInfo = task.GetTaskUIInfo();
			if (!taskUiInfo)
				continue;
			
			if (tasksToShow == "<br/>")
				tasksToShow = tasksToShow + taskUiInfo.GetTranslatedName();
			else 
				tasksToShow = tasksToShow + "<br/>" + taskUiInfo.GetTranslatedName();
		}
		
		if (tasksWidget)
		{
			if (tasksToShow != "<br/>")
				tasksWidget.SetText(string.Format(WidgetManager.Translate("<color rgba=%1>%2</color> %3", UIColors.FormatColor(UIColors.CONTRAST_COLOR), "#AR-CombatScenario_Completed_Tasks",  tasksToShow)));
			else
				tasksWidget.SetText("");
		}
	}
};