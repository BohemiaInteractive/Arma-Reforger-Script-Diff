//------------------------------------------------------------------------------------------------
//[Obsolete("Replaced with SCR_TaskMapUIComponent")]
//class SCR_MapUITask : SCR_MapUIElement
//{
//	protected bool m_bIsEnabled;	// task is hovered over & and is clickable
//	protected SCR_BaseTask m_Task;
//	protected TextWidget m_wTaskTitle;
//	protected Widget m_wAssignees;
//	protected Widget m_wMapTask;
//	Widget m_wHorizLayout;
//
//	//------------------------------------------------------------------------------
//	SCR_MapUITask InitTask(notnull SCR_BaseTask task)
//	{
//		SCR_MapDescriptorComponent descr = SCR_MapDescriptorComponent.Cast(task.FindComponent(SCR_MapDescriptorComponent));
//		if (descr)
//			m_MapItem = descr.Item();
//
//		MapDescriptorProps props = m_MapItem.GetProps();
//		props.SetIconVisible(false);
//		props.SetTextVisible(false);
//		props.Activate(true);
//
//		SCR_Faction targetFaction = SCR_Faction.Cast(task.GetTargetFaction());
//
//		ImageWidget image = ImageWidget.Cast(m_wMapTask.FindAnyWidget("TaskIconBackground"));
//		if (image)
//			task.SetTaskIconWidget(image);
//
//		image = ImageWidget.Cast(m_wMapTask.FindAnyWidget("TaskIconSymbol"));
//		if (image)
//		{
//			if (targetFaction)
//				image.SetColor(targetFaction.GetOutlineFactionColor());
//
//			task.SetWidgetIcon(image);
//		}
//
//		Widget widget = m_wMapTask.FindAnyWidget("TaskIconOutline");
//		if (widget)
//		{
//			if (targetFaction)
//				widget.SetColor(targetFaction.GetOutlineFactionColor());
//		}
//
//		if (!m_wMapTask)
//			return null;
//
//		SCR_MapUITask handler = SCR_MapUITask.Cast(m_wMapTask.FindHandler(SCR_MapUITask));
//		if (!handler)
//			return null;
//
//		SetTask(task);
//		SetEvents();
//
//		return handler;
//	}
//
//	//------------------------------------------------------------------------------
//	void SetTask(notnull SCR_BaseTask task)
//	{
//		m_Task = task;
//		SetImage(task.GetFullTaskIconName());
//
//		if (m_wTaskTitle)
//			m_Task.SetTitleWidgetText(m_wTaskTitle, m_Task.GetTaskListTaskTitle());
//
//		SetMapMarkerAssigneeCount();
//	}
//
//	//------------------------------------------------------------------------------
//	SCR_BaseTask GetTask()
//	{
//		return m_Task;
//	}
//
//	//------------------------------------------------------------------------------
//	Widget GetMapWidget()
//	{
//		return m_wMapTask;
//	}
//
//	//------------------------------------------------------------------------------
//	override bool OnMouseEnter(Widget w, int x, int y)
//	{
//		super.OnMouseEnter(w, x, y);
//
//		if (m_wMapTask)
//			m_wMapTask.SetZOrder(2);
//		
//		if (w.Type() == SizeLayoutWidget)
//			PlayHoverSound(m_sSoundHover);
//		
//		return false;
//	}
//
//	//------------------------------------------------------------------------------
//	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
//	{
//		super.OnMouseLeave(w, enterW, x, y);
//
//		if (m_wMapTask)
//			m_wMapTask.SetZOrder(0);
//
//		return false;
//	}
//
//	//------------------------------------------------------------------------------
//	override void HandlerAttached(Widget w)
//	{
//		super.HandlerAttached(w);
//
//		m_eIconType = SCR_EIconType.TASK;
//		m_wTaskTitle = TextWidget.Cast(w.FindAnyWidget("Title"));
//		m_wImage = ImageWidget.Cast(w.FindAnyWidget("TaskIcon"));
//		m_wAssignees = w.FindAnyWidget("Assignee");
//		m_wHorizLayout = w.FindAnyWidget("HorizLayout");
//		m_wMapTask = w;
//
//		ButtonWidget taskIconButton;
//		Widget overlayWidget;
//		ButtonWidget taskAssignButton
//
//		Widget widget = w.FindAnyWidget("TaskIconHover");
//		if (widget)
//		{
//			widget.SetEnabled(false);
//			widget.SetOpacity(0);
//		}
//
//		widget = w.FindAnyWidget("Border");
//		if (widget)
//		{
//			widget.SetEnabled(false);
//			widget.SetOpacity(0);
//		}
//
//		widget = w.FindAnyWidget("TaskTitleButton");
//		if (widget)
//		{
//			widget.SetVisible(false);
//		}
//
//		widget = w.FindAnyWidget("Assignee");
//		if (widget)
//		{
//			widget.SetEnabled(false);
//			widget.SetOpacity(0);
//		}
//
//		SCR_TaskSelectButton iconButtonHandler;
//		SCR_TaskOverlayButton overlayButtonHandler;
//		SCR_TaskAssignButton assignButtonHandler;
//
//		taskIconButton = ButtonWidget.Cast(w.FindAnyWidget("TaskIconButton"));
//		if (taskIconButton)
//		{
//			iconButtonHandler = SCR_TaskSelectButton.Cast(taskIconButton.FindHandler(SCR_TaskSelectButton));
//			if (iconButtonHandler)
//				iconButtonHandler.SetRootWidgetHandler(this);
//		}
//
//		overlayWidget = w.FindAnyWidget("OverlayWidget");
//		if (overlayWidget)
//		{
//			overlayButtonHandler = SCR_TaskOverlayButton.Cast(overlayWidget.FindHandler(SCR_TaskOverlayButton));
//			if (overlayButtonHandler)
//				overlayButtonHandler.SetRootWidgetHandler(this);
//		}
//
//		taskAssignButton = ButtonWidget.Cast(w.FindAnyWidget("TaskTitleButton"));
//		if (taskAssignButton)
//		{
//			assignButtonHandler = SCR_TaskAssignButton.Cast(taskAssignButton.FindHandler(SCR_TaskAssignButton));
//			if (assignButtonHandler)
//				assignButtonHandler.SetRootWidgetHandler(this);
//		}
//
//		if (GetTaskManager())
//		{
//			SCR_BaseTaskManager.s_OnTaskAssigned.Insert(GetAssignedPlayers);
//			SCR_BaseTaskManager.s_OnTaskUnassigned.Insert(GetAssignedPlayers);
//		}
//	}
//
//	//------------------------------------------------------------------------------
//	override void HandlerDeattached(Widget w)
//	{
//		if (GetTaskManager())
//		{
//			SCR_BaseTaskManager.s_OnTaskAssigned.Remove(GetAssignedPlayers);
//		 	SCR_BaseTaskManager.s_OnTaskUnassigned.Remove(GetAssignedPlayers);
//		}
//	}
//
//	//------------------------------------------------------------------------------
//	void AssignTask()
//	{
//		if (!m_Task)
//			return;
//
//		SCR_TaskNetworkComponent taskNetworkComp = SCR_TaskNetworkComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_TaskNetworkComponent));
//		if (!taskNetworkComp)
//			return;
//
//		int taskId;
//		SCR_BaseTask curTask = SCR_BaseTaskExecutor.GetLocalExecutor().GetAssignedTask();
//		if (curTask)
//		{
//			taskId = curTask.GetTaskID();
//			taskNetworkComp.AbandonTask(taskId);
//			if (curTask != m_Task)
//			{
//				taskId = m_Task.GetTaskID();
//				taskNetworkComp.RequestAssignment(taskId);
//				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.TASK_ACCEPT);
//			}
//			else
//			{
//				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.TASK_CANCELED);
//			}
//		}
//		else
//		{
//			taskId = m_Task.GetTaskID();
//			taskNetworkComp.RequestAssignment(taskId);
//			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.TASK_ACCEPT);
//		}
//	}
//
//	//------------------------------------------------------------------------------
//	void GetAssignedPlayers(SCR_BaseTask task = null)
//	{
//		if (!task || task != m_Task)
//			return;
//
//		task.UpdateMapTaskIcon();
//
//		array<SCR_BaseTaskExecutor> assignees = {};
//		m_Task.GetAssignees(assignees);
//		if (assignees.IsEmpty())
//		{
//			m_wAssignees.SetEnabled(false);
//			m_wAssignees.SetOpacity(0);
//			return;
//		}
//
//		m_wAssignees.SetEnabled(true);
//		m_wAssignees.SetOpacity(1);
//
//		SetMapMarkerAssigneeCount();
//	}
//
//	//------------------------------------------------------------------------------
//	void SetMapMarkerAssigneeCount()
//	{
//		TextWidget assigneesCount = TextWidget.Cast(m_wAssignees.FindAnyWidget("AssigneesCount"));
//		if (assigneesCount)
//			assigneesCount.SetText(m_Task.GetAssigneeCount().ToString());
//	}
//
//	//------------------------------------------------------------------------------
//	void UpdateFocusedTask()
//	{
//		SCR_UITaskManagerComponent utm = SCR_UITaskManagerComponent.GetInstance();
//		if (utm)
//			utm.Action_SelectTask(m_Task);
//	}
//
//	//------------------------------------------------------------------------------
//	void SetEvents()
//	{
//		ButtonWidget assignButton = ButtonWidget.Cast(m_wMapTask.FindAnyWidget("TaskTitleButton"));
//		if (assignButton)
//		{
//			SCR_TaskAssignButton handler = SCR_TaskAssignButton.Cast(assignButton.FindHandler(SCR_TaskAssignButton));
//			if (handler)
//				handler.GetOnMapIconClick().Insert(AssignTask);
//		}
//
//		ButtonWidget selectButton = ButtonWidget.Cast(m_wMapTask.FindAnyWidget("TaskIconButton"));
//		if (selectButton)
//		{
//			SCR_TaskSelectButton handler = SCR_TaskSelectButton.Cast(selectButton.FindHandler(SCR_TaskSelectButton));
//			if (handler)
//				handler.GetOnMapIconClick().Insert(UpdateFocusedTask);
//		}
//	}
//
//	//------------------------------------------------------------------------------
//	override vector GetPos()
//	{
//		if (!m_Task)
//			return vector.Zero;
//
//		return m_Task.GetOrigin();
//	}
//}
