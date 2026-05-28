class SCR_TaskUIColorPresetTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		SCR_ETaskState taskState = SCR_ETaskState.CREATED;
		bool isSelected = false;
		bool isAssignable = false;
		source.Get("m_eTaskState", taskState);
		source.Get("m_bIsSelected", isSelected);
		source.Get("m_bIsAssignable", isAssignable);

		title = string.Format("%1 - IsSelected: %2, IsAssignable: %3", SCR_Enum.GetEnumName(SCR_ETaskState, taskState), isSelected, isAssignable);

		return true;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sWidgetName", "SCR_TaskUIColor - Widget: %1")]
class SCR_TaskUIColor
{
	[Attribute(desc: "widget name that changes color")]
	string m_sWidgetName;

	[Attribute(desc: "If checked, uses the faction color")]
	bool m_bUseFactionColor;

	[Attribute(desc: "Color of widget")]
	ref Color m_Color;

	[Attribute("1", desc: "Opacity of widget", params: "0 1 0.01", precision: 2)]
	float m_fOpacity;

	//------------------------------------------------------------------------------------------------
	//! \returns the color of the faction or the set color, depending on the setup
	Color GetColor()
	{
		if (m_bUseFactionColor)
		{
			SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
			if (faction)
				return faction.GetFactionColor();
		}

		return m_Color;
	}
}

[BaseContainerProps(), SCR_TaskUIColorPresetTitle()]
class SCR_TaskUIColorPreset
{
	[Attribute(SCR_ETaskState.CREATED.ToString(), UIWidgets.ComboBox, "Task state", enumType: SCR_ETaskState)]
	SCR_ETaskState m_eTaskState;

	[Attribute()]
	bool m_bIsSelected;

	[Attribute("1")]
	bool m_bIsAssignable;

	[Attribute()]
	ref array<ref SCR_TaskUIColor> m_aTaskColors;

	//------------------------------------------------------------------------------------------------
	//! \param[in] widgetName
	//! \returns color setup if set for a specific widget
	SCR_TaskUIColor GetTaskColorByWidgetName(string widgetName)
	{
		foreach (SCR_TaskUIColor taskUIColor : m_aTaskColors)
		{
			if (taskUIColor && taskUIColor.m_sWidgetName == widgetName)
				return taskUIColor;
		}

		return null;
	}
}

[BaseContainerProps()]
class SCR_TaskUIColorPresetList
{
	[Attribute(desc:"Task UI color presets")]
	protected ref array<ref SCR_TaskUIColorPreset> m_aTaskUIColorPresets;

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \return true if the preset is set for the task
	bool HasTaskUIColorPreset(notnull SCR_Task task)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] taskState
	//! \param[in] isSelected
	//! \param[in] isAssignable
	//! \returns the task UI colo preset if it is found by the parameter settings
	SCR_TaskUIColorPreset GetTaskUIColorPreset(SCR_ETaskState taskState, bool isSelected = false, bool isAssignable = true)
	{
		if (!m_aTaskUIColorPresets)
			return null;

		foreach (SCR_TaskUIColorPreset preset : m_aTaskUIColorPresets)
		{
			if (preset &&
				preset.m_eTaskState == taskState &&
				preset.m_bIsSelected == isSelected &&
				preset.m_bIsAssignable == isAssignable)
				return preset;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] widget
	//! \param[in] taskState
	//! \param[in] isSelected
	//! \param[in] isAssignable
	void SetTaskWidgetColor(Widget widget, SCR_ETaskState taskState, bool isSelected = false, bool isAssignable = true)
	{
		if (!widget)
			return;

		SCR_TaskUIColorPreset preset = GetTaskUIColorPreset(taskState, isSelected, isAssignable);
		if (!preset)
			return;

		// try find widget color if is defined
		SCR_TaskUIColor presetTaskColor = preset.GetTaskColorByWidgetName(widget.GetName());
		if (!presetTaskColor)
			return;

		Color color = presetTaskColor.GetColor();
		if (color)
			widget.SetColor(color);

		widget.SetOpacity(presetTaskColor.m_fOpacity);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \return task state
	SCR_ETaskState GetUITaskState(notnull SCR_Task task)
	{
		SCR_ETaskState taskState = task.GetTaskState();

		if (!SCR_Enum.HasPartialFlag(SCR_ETaskState.COMPLETED | SCR_ETaskState.CANCELLED | SCR_ETaskState.FAILED, taskState))
		{
			if (IsTaskAssigned(task))
				taskState = SCR_ETaskState.ASSIGNED;
			else
				taskState = SCR_ETaskState.CREATED;
		}

		return taskState;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \returns true if task is assigned to a local player or it should look that way
	bool IsTaskAssigned(notnull SCR_Task task)
	{
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
		if (!player)
			return false;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		if (task.IsTaskAssignedTo(player) || (faction && faction.IsPlayerCommander(playerID) && task.GetTaskAssigneeCount() > 0))
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \returns true if task can be assigned to a local player or it should look that way
	bool CanTaskBeAssigned(notnull SCR_Task task)
	{
		if (SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			return true;

		SCR_PlayerControllerGroupComponent playerControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerControllerGroupComponent)
			return false;

		SCR_TaskExecutor groupExecutor = SCR_TaskExecutor.FromGroup(playerControllerGroupComponent.GetGroupID());
		if (!groupExecutor)
			return false;

		return SCR_TaskSystem.GetInstance().CanTaskBeAssignedTo(task, groupExecutor);
	}
}

[BaseContainerProps(configRoot: true)]
class SCR_RequestedTaskUIColorPresetList : SCR_TaskUIColorPresetList
{
	//------------------------------------------------------------------------------------------------
	override bool HasTaskUIColorPreset(notnull SCR_Task task)
	{
		return task.IsInherited(SCR_BaseRequestedTaskEntity);
	}
}

[BaseContainerProps(configRoot: true)]
class SCR_ObjectiveTaskUIColorPresetList : SCR_TaskUIColorPresetList
{
	//------------------------------------------------------------------------------------------------
	override bool HasTaskUIColorPreset(notnull SCR_Task task)
	{
		return !task.IsInherited(SCR_BaseRequestedTaskEntity);
	}
}
