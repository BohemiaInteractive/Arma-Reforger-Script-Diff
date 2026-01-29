[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_KillfeedReceiveTypeEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
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
		
		EKillFeedReceiveType killFeedReceiveType = sender.GetReceiveKillFeedType();
		array<ref SCR_NotificationKillfeedreceiveTypeName> killFeedReceiverTypeNames = new array<ref SCR_NotificationKillfeedreceiveTypeName>;
		int count = sender.GetKillFeedReceiveTypeNames(killFeedReceiverTypeNames);
		
		for(int i = 0; i < count; i++)
        {
           	if (killFeedReceiverTypeNames[i].GetKillfeedReceiveType() == killFeedReceiveType)
				return SCR_BaseEditorAttributeVar.CreateInt(i);
        }
		
		Print("SCR_KillfeedReceiveTypeEditorAttribute:  m_aKillfeedreceiveTypeNames in SCR_NotificationSenderComponent is missing EKillFeedReceiveType!", LogLevel.WARNING);
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
		SCR_NotificationSenderComponent sender;
		if (gameMode)
			sender = SCR_NotificationSenderComponent.Cast(gameMode.FindComponent(SCR_NotificationSenderComponent));
		
		if (sender)
		{
			m_aValues.Clear();
			array<ref SCR_NotificationKillfeedreceiveTypeName> killFeedReceiverTypeNames = new array<ref SCR_NotificationKillfeedreceiveTypeName>;
			sender.GetKillFeedReceiveTypeNames(killFeedReceiverTypeNames);
			SCR_EditorAttributeFloatStringValueHolder value;
			
			foreach (SCR_NotificationKillfeedreceiveTypeName killfeedReceiverName: killFeedReceiverTypeNames)
			{
				value = new SCR_EditorAttributeFloatStringValueHolder();
				value.SetName(killfeedReceiverName.GetName());
				value.SetFloatValue(killfeedReceiverName.GetKillfeedReceiveType());
				
				m_aValues.Insert(value);
			}
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
		
		sender.SetReceiveKillFeedType(m_aValues[var.GetInt()].GetFloatValue(), playerID);
	}
};