[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_FriendlyFireKillfeedTypeEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		BaseGameMode gamemode = BaseGameMode.Cast(item);
		if (!gamemode)
			return null;
		
		SCR_NotificationSenderComponent sender = SCR_NotificationSenderComponent.Cast(gamemode.FindComponent(SCR_NotificationSenderComponent));
		if (!sender)
			return null;
		
		SCR_EFriendlyFireKillFeedType friendlyFireKillFeedReceiveType = sender.GetFriendlyFireKillFeedType();
		array<ref SCR_NotificationFriendlyFireKillfeedTypeName> friendlyFireKillFeedTypeNames = {};
		int count = sender.GetFriendlyFireKillFeedTypeNames(friendlyFireKillFeedTypeNames);
		
		for (int i = 0; i < count; i++)
        {
           	if (friendlyFireKillFeedTypeNames[i].GetFriendlyFireKillfeedType() == friendlyFireKillFeedReceiveType)
				return SCR_BaseEditorAttributeVar.CreateInt(i);
        }
		
		Print("SCR_FriendlyFireKillfeedTypeEditorAttribute:  m_aFriendlyFireKillfeedTypeNames in SCR_NotificationSenderComponent is missing SCR_EFriendlyFireKillFeedType!", LogLevel.WARNING);
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		FillValues();
		outEntries.Insert(new SCR_BaseEditorAttributeFloatStringValues(m_aValues));
		return outEntries.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillValues()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_NotificationSenderComponent sender = SCR_NotificationSenderComponent.Cast(gameMode.FindComponent(SCR_NotificationSenderComponent));
		if (!sender)
			return;
		
		m_aValues.Clear();
		array<ref SCR_NotificationFriendlyFireKillfeedTypeName> friendlyFireKillFeedTypeNames = {};
		sender.GetFriendlyFireKillFeedTypeNames(friendlyFireKillFeedTypeNames);
		SCR_EditorAttributeFloatStringValueHolder value;
		
		foreach (SCR_NotificationFriendlyFireKillfeedTypeName friendlyFirekillfeedName: friendlyFireKillFeedTypeNames)
		{
			value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetName(friendlyFirekillfeedName.GetName());
			value.SetFloatValue(friendlyFirekillfeedName.GetFriendlyFireKillfeedType());
			
			m_aValues.Insert(value);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		BaseGameMode gamemode = BaseGameMode.Cast(item);
		if (!gamemode)
			return;
		
		SCR_NotificationSenderComponent sender = SCR_NotificationSenderComponent.Cast(gamemode.FindComponent(SCR_NotificationSenderComponent));
		if (!sender)
			return;
		
		if (m_aValues.IsEmpty())
		{
			FillValues();
			if (m_aValues.IsEmpty())
				return;
		}
		
		sender.SetFriendlyFireKillFeedType(m_aValues[var.GetInt()].GetFloatValue(), playerID);
	}
}