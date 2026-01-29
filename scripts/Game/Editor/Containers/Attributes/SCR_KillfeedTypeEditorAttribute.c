[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_KillfeedTypeEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
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
		
		EKillFeedType killFeedType = sender.GetKillFeedType();
		array<ref SCR_NotificationKillfeedTypeName> killFeedTypeNames = new array<ref SCR_NotificationKillfeedTypeName>;
		int count = sender.GetKillFeedTypeNames(killFeedTypeNames);
		
		for(int i = 0; i < count; i++)
        {
           	if (killFeedTypeNames[i].GetKillfeedType() == killFeedType)
				return SCR_BaseEditorAttributeVar.CreateInt(i);
        }
		
		Print("SCR_KillfeedReceiveTypeEditorAttribute:  m_aKillfeedTypeNames in SCR_NotificationSenderComponent is missing EKillFeedType!", LogLevel.WARNING);
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
			array<ref SCR_NotificationKillfeedTypeName> killFeedTypeNames = new array<ref SCR_NotificationKillfeedTypeName>;
			sender.GetKillFeedTypeNames(killFeedTypeNames);
			SCR_EditorAttributeFloatStringValueHolder value;
			
			foreach (SCR_NotificationKillfeedTypeName killfeedName: killFeedTypeNames)
			{
				value = new SCR_EditorAttributeFloatStringValueHolder();
				value.SetName(killfeedName.GetName());
				value.SetFloatValue(killfeedName.GetKillfeedType());
				
				m_aValues.Insert(value);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_KillfeedReceiveTypeEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_FriendlyFireKillfeedTypeEditorAttribute);
		}
				
		manager.SetAttributeEnabled(SCR_KillfeedReceiveTypeEditorAttribute, var && var.GetInt() != 0);
		manager.SetAttributeEnabled(SCR_FriendlyFireKillfeedTypeEditorAttribute, var && var.GetInt() != 0);
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

		sender.SetKillFeedType(m_aValues[var.GetInt()].GetFloatValue(), playerID);
	}
};