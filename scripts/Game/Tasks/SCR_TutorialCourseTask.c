//------------------------------------------------------------------------------------------------
//class SCR_TutorialCourseTaskClass: SCR_TaskClass
//{
//};
//
//------------------------------------------------------------------------------------------------
//class SCR_TutorialCourseTask : SCR_Task
//{
//	static const string TASK_COMPLETED_SUFIX = "#AR-Tutorial_TaskCompletedSuffix";
//	
//	SCR_ETutorialCourses m_eCourse;
//	protected bool m_bComplete;
//	protected ref ScriptInvoker m_OnClicked;
//	protected ref ScriptInvoker m_OnWidgetSet;
//	
//	//TODO> HIDE method?
//	//------------------------------------------------------------------------------------------------
//	void SetSelected(bool selected)
//	{
//		Widget focus = GetTaskIconkWidget();
//		if (!focus)
//			return;
//		
//		focus = focus.GetParent();
//		if (!focus)
//			return;
//		
//		focus = focus.FindAnyWidget("TaskIconFocus");
//		if (focus)
//			focus.SetVisible(selected);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	ScriptInvoker GetOnWidgetSet()
//	{
//		if (!m_OnWidgetSet)
//			m_OnWidgetSet = new ScriptInvoker;
//		
//		return m_OnWidgetSet;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[Obsolete("Use methods from SCR_MapTaskUIComponent")]
//	ScriptInvoker GetOnClicked()
//	{	
//		if (!m_wMapTaskIcon)
//			return null;
//		
//		Widget w = m_wMapTaskIcon.GetParent();
//		while (w)
//		{
//			if (w.GetName() == "TaskIconButton")
//				break;
//			
//			w = w.GetParent();
//		}
//		
//		if (!w)
//			return null;
//		
//		SCR_TaskSelectButton taskSelectButton = SCR_TaskSelectButton.Cast(w.FindHandler(SCR_TaskSelectButton));
//		if (!taskSelectButton)
//			return null;
//		
//		//##TODO
//		//taskSelectButton.GetOnMapIconClick().Insert(OnSelectButtonClicked);
//		m_OnClicked = new ScriptInvoker;
//		
//		return m_OnClicked;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void OnSelectButtonClicked()
//	{
//		if (m_OnClicked)
//			m_OnClicked.Invoke(this);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void CompleteCourseTask()
//	{
//		if (m_bComplete)
//			return;
//		
//		m_bComplete = true;
//		SetTaskName(GetTaskName() + " " + TASK_COMPLETED_SUFIX);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetIconName(string iconName)
//	{
//		//m_sMapIconName = iconName;
//		//m_sTaskListIconName = iconName;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void UpdateIcon()
//	{
//		if (!m_wMapTaskIcon)
//			return;
//		
//		ImageWidget outline = ImageWidget.Cast(m_wMapTaskIcon.GetParent().FindAnyWidget("TaskIconOutline"));
//		ImageWidget background = ImageWidget.Cast(m_wMapTaskIcon.GetParent().FindAnyWidget("TaskIconBackground"));
//		ImageWidget hover = ImageWidget.Cast(m_wMapTaskIcon.GetParent().FindAnyWidget("TaskIconHover"));
//		ImageWidget symbol = ImageWidget.Cast(m_wMapTaskIcon.GetParent().FindAnyWidget("TaskIconSymbol"));
//		
//		if (!symbol || !outline || !background || !hover)
//			return;
//
//		if (IsAssignedToLocalPlayer())
//		{
//			background.SetColor(UIColors.CONTRAST_COLOR);
//		}
//		else
//		{
//			if (m_bComplete)
//			{
//				background.SetColor(Color.FromSRGBA(150,224,246,255));
//				symbol.SetColor(Color.FromSRGBA(16,59,95,200));
//			}
//			else
//			{
//				background.SetColor(m_TargetFaction.GetFactionColor());
//				outline.SetVisible(true);
//			}
//		}
//		
//		symbol.LoadImageFromSet(0, m_sIconImageset, m_sMapIconName);
//		
//		if (!m_bComplete)
//			return;
//		
//		ImageWidget finishedIcon = ImageWidget.Cast(m_wMapTaskIcon.GetParent().FindAnyWidget("TaskIconFinished"));
//		if (finishedIcon)
//			finishedIcon.SetVisible(true);
//		
//		ImageWidget finishedBG = ImageWidget.Cast(m_wMapTaskIcon.GetParent().FindAnyWidget("TaskIconFinishedBG"));
//		if (finishedBG)
//			finishedBG.SetVisible(true);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTaskIconWidget(Widget w)
//	{
//		if (m_OnWidgetSet)
//			m_OnWidgetSet.Invoke(this);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//TODO: Get rid of redundant Update methods. One should be enough for tutorial, as we don't use priorities.
//	void UpdatePriorityMapTaskIcon()
//	{
//		UpdateIcon();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void UpdateMapTaskIcon()
//	{
//		UpdateIcon();
//	}
//}