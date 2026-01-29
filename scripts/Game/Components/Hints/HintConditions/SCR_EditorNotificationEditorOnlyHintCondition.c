[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorNotificationEditorOnlyHintCondition : SCR_BaseEditorHintCondition
{
	//------------------------------------------------------------------------------------------------
	protected void OnNotification(SCR_NotificationData data)
	{
		if (!data)
			return;
		
		if (data.GetNotificationReceiverType() != ENotificationReceiver.GM_ONLY && data.GetNotificationReceiverType() != ENotificationReceiver.LOCAL_GM_ONLY)
			return; 
		
		//~ Add 1 frame delay as in many causes the notifcation is shown after an attribute is changed and it will need to wait until menu is properly closed
		GetGame().GetCallqueue().CallLater(DelayedShowHint);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedShowHint()
	{
		Activate();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_NotificationsComponent notificationsManager = SCR_NotificationsComponent.GetInstance();
		if (notificationsManager)
			notificationsManager.GetOnNotification().Insert(OnNotification);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_NotificationsComponent notificationsManager = SCR_NotificationsComponent.GetInstance();
		if (notificationsManager)
			notificationsManager.GetOnNotification().Remove(OnNotification);
	}
}
