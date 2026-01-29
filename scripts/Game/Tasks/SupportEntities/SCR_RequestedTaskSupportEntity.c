//------------------------------------------------------------------------------------------------
//[Obsolete(), EntityEditorProps(category: "GameScripted/Tasks", description: "Requested task support entity.", color: "0 0 255 255")]
//class SCR_RequestedTaskSupportEntityClass: SCR_BaseTaskSupportEntityClass
//{
//};

//------------------------------------------------------------------------------------------------
//[Obsolete()]
//class SCR_RequestedTaskSupportEntity : SCR_BaseTaskSupportEntity
//{
//	[Attribute("{75C912A1C89BE6C2}UI/layouts/WidgetLibrary/Buttons/WLib_ButtonText.layout")]
//	protected ResourceName m_sUIRequestButtonResource;
//	
//	[Attribute("0", UIWidgets.ComboBox, "Request type", "", ParamEnumArray.FromEnum(SCR_EUIRequestType))]
//	protected SCR_EUIRequestType m_eRequestType;
//	
//	[Attribute("", UIWidgets.LocaleEditBox, "Request button text.")]
//	protected LocalizedString m_sRequestButtonText;
//	
//	protected SCR_RequestedTask m_LocallyRequestedTask;
//	
//	//------------------------------------------------------------------------------------------------
//	void SetLocallyRequestedTask(notnull SCR_RequestedTask requestedTask)
//	{
//		m_LocallyRequestedTask = requestedTask;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	SCR_RequestedTask GetLocallyRequestedTask()
//	{
//		return m_LocallyRequestedTask;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetRequester(notnull SCR_RequestedTask task, SCR_BaseTaskExecutor requester)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		int taskID, requesterID;
//		taskID = task.GetTaskID();
//		requesterID = SCR_BaseTaskExecutor.GetTaskExecutorID(requester);
//		
//		Rpc(RPC_SetRequester, taskID, requesterID);
//		RPC_SetRequester(taskID, requesterID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_SetRequester(int taskID, int requesterID)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_RequestedTask task = SCR_RequestedTask.Cast(GetTaskManager().GetTask(taskID));
//		if (!task)
//			return;
//		
//		SCR_BaseTaskExecutor requester = SCR_BaseTaskExecutor.GetTaskExecutorByID(requesterID);
//		if (!requester)
//			return;
//		
//		task.SetRequester(requester);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	vector GetRequestPosition()
//	{
//		// By default we take map to world position - from context menu
//		
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	bool CanRequest()
//	{
//		if (!GetTaskManager())
//			return false;
//		
//		SCR_RequestedTaskSupportEntity supportEntity = SCR_RequestedTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_RequestedTaskSupportEntity));
//		if (!supportEntity)
//			return false;
//		
//		return (!supportEntity.GetLocallyRequestedTask());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetRequestButtonText(notnull TextWidget textWidget)
//	{
//		textWidget.SetTextFormat(m_sRequestButtonText);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	/*void OnContextualEntryShow(SCR_MapContextualMenuEntry entry, Widget button)
//	{
//		TextWidget textWidget = TextWidget.Cast(button.FindAnyWidget("Text"));
//		if (!textWidget)
//			return;
//		
//		SetRequestButtonText(textWidget);
//	}*/
//	
//	//------------------------------------------------------------------------------------------------
//	void Request()
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected override void RegisterContextualMenuCallbacks()
//	{
//		if (m_sRequestButtonText.IsEmpty())
//			return;
//		
//		SCR_MapRadialUI radialMenu = SCR_MapRadialUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapRadialUI));
//		if (!radialMenu)
//			return;
//				
//		SCR_MapMenuRequestedTaskEntry entry = new SCR_MapMenuRequestedTaskEntry();
//		if (!entry)
//			return;
//		
//		entry.SetName(m_sRequestButtonText);
//		entry.SetSupportClass(this);
//		entry.GetOnPerform().Insert(Request);
//		//entry.m_OnShow.Insert(OnContextualEntryShow);
//		
//		radialMenu.InsertCustomRadialEntry(entry);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	Widget CreateButtonAndSetPadding(Widget parentWidget, notnull inout array<Widget> widgets, float left = 4, float top = 4, float right = 4, float bottom = 4)
//	{
//		Widget requestButtonLayout = GetGame().GetWorkspace().CreateWidgets(m_sUIRequestButtonResource, parentWidget);
//		if (!requestButtonLayout)
//			return null;
//		
//		ButtonWidget requestButton = ButtonWidget.Cast(requestButtonLayout.FindAnyWidget("Button"));
//		if (requestButton)
//		{
//			SCR_UIRequestEvacTaskComponent requestTaskComponent = SCR_UIRequestEvacTaskComponent.Cast(requestButton.FindHandler(SCR_UIRequestEvacTaskComponent));
//			if (requestTaskComponent)
//				requestTaskComponent.SetRequestType(m_eRequestType);
//		}
//		
//		VerticalLayoutSlot.SetPadding(requestButtonLayout, left, top, right, bottom);
//		
//		TextWidget textWidget = TextWidget.Cast(requestButtonLayout.FindAnyWidget("Text"));
//		if (textWidget)
//			SetRequestButtonText(textWidget);
//		
//		widgets.Insert(requestButtonLayout);
//		
//		return requestButtonLayout;
//	}
//};