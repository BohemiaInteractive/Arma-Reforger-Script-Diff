class SCR_ConflictTaskListEntryUIComponent : SCR_TaskListEntryUIComponent
{
	//------------------------------------------------------------------------------------------------
	protected override void HandleTaskState(SCR_ETaskState state, bool forceRefresh = true)
	{
		super.HandleTaskState(state, forceRefresh);

		UpdateTaskWidgetColorsFromPreset();
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateTaskIconColors(Color backgroundColor = null, Color iconColor = null, Color outlineColor = null)
	{
		super.UpdateTaskIconColors(backgroundColor, iconColor, outlineColor);

		UpdateTaskWidgetColorsFromPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTaskWidgetColorsFromPreset()
	{
		if (!m_TaskManager || !m_Task)
			return;

		array<ref SCR_TaskUIColorPresetList> taskUIColorPresetList = m_TaskManager.GetTaskUIColorPresetList();
		if (!taskUIColorPresetList)
			return;

		bool isSelected = m_Task == m_TaskManager.GetSelectedTask();
		SCR_ETaskState taskState;
		bool isAssignable;

		foreach (SCR_TaskUIColorPresetList presetList : taskUIColorPresetList)
		{
			if (!presetList || !presetList.HasTaskUIColorPreset(m_Task))
				continue;

			taskState = presetList.GetUITaskState(m_Task);
			isAssignable = presetList.CanTaskBeAssigned(m_Task);

			presetList.SetTaskWidgetColor(m_Widgets.m_wTaskIconBackground, taskState, isSelected, isAssignable);
			presetList.SetTaskWidgetColor(m_Widgets.m_wTaskIconSymbol, taskState, isSelected, isAssignable);
			presetList.SetTaskWidgetColor(m_Widgets.m_wTaskIconOutline, taskState, isSelected, isAssignable);
			presetList.SetTaskWidgetColor(m_Widgets.m_wIconOverlay, taskState, isSelected, isAssignable);
			presetList.SetTaskWidgetColor(m_Widgets.m_wTaskTitle, taskState, isSelected, isAssignable);
		}
	}
}
