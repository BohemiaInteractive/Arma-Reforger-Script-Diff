
//------------------------------------------------------------------------------------------------
//! Tutorial fast travel map menu
class SCR_TutorialFastTravelMapMenuUI : ChimeraMenuBase
{
	protected SCR_MapEntity m_MapEntity;
	protected SCR_TutorialFastTravelSpinBox m_SpinboxComp;
	protected SCR_Task m_SelectedTask;

	//------------------------------------------------------------------------------------------------
	protected SCR_TutorialTaskMapUIComponent GetTaskMapUIComponent(SCR_Task task)
	{
		if (!task || !m_MapEntity)
			return null;

		SCR_MapUIElementContainer mapUIElementContainer = SCR_MapUIElementContainer.Cast(m_MapEntity.GetMapUIComponent(SCR_MapUIElementContainer));
		if (!mapUIElementContainer)
			return null;

		return SCR_TutorialTaskMapUIComponent.Cast(mapUIElementContainer.GetTaskFromID(task.GetTaskID()));
	}

	//------------------------------------------------------------------------------------------------
	protected void ConfirmSelection()
	{
		if (!m_SelectedTask)
			return;

		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;

		tutorial.FastTravelToCourse(m_SelectedTask.GetTaskID().ToInt());

		MenuManager menuMan = GetGame().GetMenuManager();
		if (menuMan)
			menuMan.FindMenuByPreset(ChimeraMenuPreset.TutorialFastTravel).Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectTask(SCR_Task task)
	{
		if (!task)
			return;

		if (m_SelectedTask)
		{
			SCR_TutorialTaskMapUIComponent taskUI = GetTaskMapUIComponent(m_SelectedTask);
			if (taskUI)
				taskUI.SetSelected(false);
		}

		m_SelectedTask = task;
		SCR_TutorialTaskMapUIComponent taskUI = GetTaskMapUIComponent(m_SelectedTask);
		if (taskUI)
			taskUI.SetSelected(true);

		vector position = task.GetOrigin();
		if (m_MapEntity)
			m_MapEntity.ZoomPanSmooth(1.5, position[0], position[2]);

		m_SpinboxComp.SetCurrentItem(m_SpinboxComp.GetItemIndex(task));
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		if (!m_MapEntity)
			return;

		MapConfiguration mapConfigFullscreen = m_MapEntity.SetupMapConfig(EMapEntityMode.TUTORIALFASTTRAVEL, "{727AAAACDCBB4EFE}Configs/Map/TutorialFastTravelMap.conf", GetRootWidget());
		m_MapEntity.OpenMap(mapConfigFullscreen);

		m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);

		SetupSpinbox();

		InputManager inputMan = GetGame().GetInputManager();
		if (!inputMan)
			return;

		inputMan.AddActionListener("TutorialFastTravelMapMenuClose", EActionTrigger.DOWN, Close);
		inputMan.AddActionListener("TutorialFastTravelMapMenuConfirm", EActionTrigger.DOWN, ConfirmSelection);

		SCR_TaskManagerUIComponent taskManagerUI = SCR_TaskManagerUIComponent.GetInstance();
		if (taskManagerUI)
			taskManagerUI.GetOnTaskSelected().Insert(SelectTask);
	}

	//------------------------------------------------------------------------------------------------
	void OnTaskClick(SCR_Task task)
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;

		InputManager inputMan = GetGame().GetInputManager();
		if (!inputMan)
			return;

		if (inputMan.IsUsingMouseAndKeyboard())
		{
			tutorial.FastTravelToCourse(task.GetTaskID().ToInt());

			MenuManager menuMan = GetGame().GetMenuManager();
			if (menuMan)
				menuMan.FindMenuByPreset(ChimeraMenuPreset.TutorialFastTravel).Close();

			return;
		}

		SelectTask(task);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupSpinbox()
	{
		Widget spinBox = GetRootWidget().FindAnyWidget("FastTravelSpinBox");
		if (!spinBox)
			return;

		m_SpinboxComp = SCR_TutorialFastTravelSpinBox.Cast(spinBox.FindHandler(SCR_TutorialFastTravelSpinBox));
		if (!m_SpinboxComp)
			return;

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		array<SCR_Task> tasks = {};
		taskSystem.GetTasks(tasks);

		foreach (SCR_Task task : tasks)
		{
			if (task.GetTaskVisibility() == SCR_ETaskVisibility.NONE)
				continue;

			m_SpinboxComp.AddItem(task.GetTaskName(), false, task);
		}

		m_SpinboxComp.m_OnChanged.Insert(OnSpinboxChanged);
	}

	//------------------------------------------------------------------------------------------------
	void OnSpinboxChanged()
	{
		SelectTask(SCR_Task.Cast(m_SpinboxComp.GetCurrentItemData()));
	}

	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		m_MapEntity.GetOnMapOpen().Remove(OnMapOpen);

		m_MapEntity.ZoomPanSmooth(0.35, m_MapEntity.GetMapSizeX()/ 2, m_MapEntity.GetMapSizeY()* 0.5);

		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_MAP_OPEN);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
		if (m_MapEntity)
		{
			m_MapEntity.CloseMap();

			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_MAP_CLOSE);
		}

		InputManager inputMan = GetGame().GetInputManager();
		if (!inputMan)
			return;

		inputMan.RemoveActionListener("TutorialFastTravelMapMenuClose", EActionTrigger.DOWN, Close);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuInit()
	{
		if (!m_MapEntity)
			m_MapEntity = SCR_MapEntity.GetMapInstance();
	}
}
