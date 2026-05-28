class SCR_ConflictTaskMapUIComponent : SCR_TaskMapUIComponent
{
	//------------------------------------------------------------------------------------------------
	protected override void OnDisplayDataChanged()
	{
		super.OnDisplayDataChanged();

		UpdateTaskWidgetColorsFromPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnTaskSelected(SCR_Task task)
	{
		super.OnTaskSelected(task);

		UpdateTaskWidgetColorsFromPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnTaskAssigneeAdded(SCR_Task task, SCR_TaskExecutor assignee, int requesterID)
	{
		super.OnTaskAssigneeAdded(task, assignee, requesterID);

		UpdateTaskWidgetColorsFromPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected override void UpdateTaskState(SCR_Task task, SCR_ETaskState state)
	{
		super.UpdateTaskState(task, state);

		if (m_Task != task)
			return;

		UpdateTaskWidgetColorsFromPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnTaskGroupChanged(SCR_Task task, int id)
	{
		super.OnTaskGroupChanged(task, id);

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
		}
	}
}
