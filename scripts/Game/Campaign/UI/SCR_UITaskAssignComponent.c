//class SCR_UITaskAssignComponent : ScriptedWidgetComponent
//{
//	protected int m_iTaskID = SCR_BaseTask.INVALID_TASK_ID;
//	protected SCR_ETaskButtonActions m_EAction;
//	
//	//------------------------------------------------------------------------------------------------
//	//! \param[in] action
//	void SetAction(SCR_ETaskButtonActions action)
//	{
//		m_EAction = action;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns task ID
//	int GetTaskID()
//	{
//		return m_iTaskID;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Sets task ID
//	void SetTaskID(int taskID)
//	{
//		m_iTaskID = taskID;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[Obsolete("Use new SCR_TaskManagerUIComponent method instead")]
//	override bool OnMouseEnter(Widget w, int x, int y)
//	{	
//		//SCR_UITaskManagerComponent.PanMapToTask(m_iTaskID);
//		
//		return false;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[Obsolete("Use new Task System instead")]
//	override bool OnClick(Widget w, int x, int y, int button)
//	{
//		if (m_iTaskID == -1)
//			return false;
//		// Find local player controller
//		PlayerController playerController = GetGame().GetPlayerController();
//		if (!playerController)
//			return false;
//		
//		// Find task network component to send RPC to server
//		SCR_TaskNetworkComponent taskNetworkComponent = SCR_TaskNetworkComponent.Cast(playerController.FindComponent(SCR_TaskNetworkComponent));
//		if (!taskNetworkComponent)
//			return false;
//		
//		switch (m_EAction)
//		{
//			case SCR_ETaskButtonActions.ASSIGN_TASK:
//				taskNetworkComponent.RequestAssignment(m_iTaskID);
//				break;
//
//			case SCR_ETaskButtonActions.ABANDON_TASK:
//				taskNetworkComponent.AbandonTask(m_iTaskID);
//				break;
//
//			case SCR_ETaskButtonActions.CANCEL_TASK:
//				taskNetworkComponent.CancelTask(m_iTaskID);
//				break;
//		}
//		
//		return false;
//	}
//}
