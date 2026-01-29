[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationTaskStateChanged : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{		
		int taskID, factionID;
		data.GetParams(taskID, factionID);
		
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(taskID));
		if (!entity)
			return m_info.GetName();
		
		SCR_EditorTask task = SCR_EditorTask.Cast(entity.GetOwner());
		if (!task)
			return m_info.GetName();
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return m_info.GetName();
		
		Faction faction = factionManager.GetFactionByIndex(factionID);
		if (!faction)
			return m_info.GetName();
		
		data.SetNotificationTextEntries(task.GetLocationName(), entity.GetDisplayName(), faction.GetUIInfo().GetName());		
		return super.GetText(data);
	}
	
	override void SetPosition(SCR_NotificationData data)
	{
		if (!CanSetPosition(data))
			return;
		
		int taskID, factionID;
		data.GetParams(taskID, factionID);
		
		SetPositionDataEditableEntity(taskID, data);
	}
	
	override bool MergeParam1With2()
	{
		return true;
	}
};