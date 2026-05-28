class SCR_ConflictCurrentTaskUIComponent : SCR_CurrentTaskUIComponent
{
	protected SCR_TaskManagerUIComponent m_TaskManager;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_TaskManager = SCR_TaskManagerUIComponent.GetInstance();
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateTask(SCR_Task task)
	{
		super.UpdateTask(task);

		if (!task)
		{
			m_Widgets.m_wTaskIconBackground.SetColor(UIColors.NEUTRAL_INFORMATION);
			return;
		}

		UpdateTaskWidgetColorsFromPreset(task);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTaskWidgetColorsFromPreset(notnull SCR_Task task)
	{
		if (!m_TaskManager)
			return;

		array<ref SCR_TaskUIColorPresetList> taskUIColorPresetList = m_TaskManager.GetTaskUIColorPresetList();
		if (!taskUIColorPresetList)
			return;

		bool isSelected = task == m_TaskManager.GetSelectedTask();
		SCR_ETaskState taskState;
		bool isAssignable;

		foreach (SCR_TaskUIColorPresetList presetList : taskUIColorPresetList)
		{
			if (!presetList || !presetList.HasTaskUIColorPreset(task))
				continue;

			taskState = presetList.GetUITaskState(task);
			isAssignable = presetList.CanTaskBeAssigned(task);

			presetList.SetTaskWidgetColor(m_Widgets.m_wTaskIconBackground, taskState, isSelected, isAssignable);
			presetList.SetTaskWidgetColor(m_Widgets.m_wTaskIconSymbol, taskState, isSelected, isAssignable);
			presetList.SetTaskWidgetColor(m_Widgets.m_wTaskIconOutline, taskState, isSelected, isAssignable);
			presetList.SetTaskWidgetColor(m_Widgets.m_wIconOverlay, taskState, isSelected, isAssignable);
		}
	}
}
