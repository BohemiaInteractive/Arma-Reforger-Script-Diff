class SCR_TutorialTaskMapUIComponent : SCR_TaskMapUIComponent
{
	protected SCR_TutorialFastTravelMapMenuUI m_TutorialMapMenu;
	
	[Attribute(UIColors.GetColorAttribute(Color.FromSRGBA(137, 137, 137, 255)), UIWidgets.ColorPicker, desc: "Color of finished task")]
	protected ref Color m_CourseFinishedColor;
	
	protected const string m_sWidgetNameTaskIconFinished = "TaskIconFinished";
	protected const string m_sWidgetNameTaskIconFinishedBG = "TaskIconFinishedBG";
	protected const string m_sWidgetNameTaskIconFocus = "TaskIconFocus";

	//------------------------------------------------------------------------------------------------
	override void InitTask(notnull SCR_Task task)
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;

		m_Task = task;

		m_TaskSystem = SCR_TaskSystem.GetInstance();
		if (!m_TaskSystem && !m_TaskSystem.IsTaskVisibleOnMap(task))
			return;

		SCR_Faction faction = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("US"));
		if (!faction)
			return;

		m_FactionColor = faction.GetFactionColor();
		m_BackgroundColor = m_FactionColor;
		m_OutlineColor = faction.GetOutlineFactionColor();
		m_IconColor = m_OutlineColor;

		UpdateTaskIconColors(m_FactionColor);

		if (SCR_Enum.HasFlag(tutorial.GetFinishedCourses(), task.GetTaskID().ToInt()))
		{
			m_FactionColor = m_CourseFinishedColor;

			Widget w = m_wRoot.FindAnyWidget(m_sWidgetNameTaskIconFinished);
			if (w)
				w.SetVisible(true);

			w = m_wRoot.FindAnyWidget(m_sWidgetNameTaskIconFinishedBG);
			if (w)
				w.SetVisible(true);

			UpdateTaskIconColors(m_FactionColor);
		}

		SetTask(task);
	}

	//------------------------------------------------------------------------------------------------
	void SetSelected(bool select)
	{
		Widget focus = m_wRoot.FindAnyWidget(m_sWidgetNameTaskIconFocus);
		if (!focus)
			return;

		focus.SetVisible(select);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		MenuManager menuMan = GetGame().GetMenuManager();
		if (!menuMan)
			return;

		m_TutorialMapMenu = SCR_TutorialFastTravelMapMenuUI.Cast(menuMan.FindMenuByPreset(ChimeraMenuPreset.TutorialFastTravel));
		if (!m_TutorialMapMenu)
			return;
	}

	//------------------------------------------------------------------------------------------------
	// Sets task as selected on task icon click
	override protected void OnTaskIconClicked()
	{
		super.OnTaskIconClicked();

		if (!m_TutorialMapMenu)
			return;

		m_TutorialMapMenu.OnTaskClick(m_Task);
	}
}
