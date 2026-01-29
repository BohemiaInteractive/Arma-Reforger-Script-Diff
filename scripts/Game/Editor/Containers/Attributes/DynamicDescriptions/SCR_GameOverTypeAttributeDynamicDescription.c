/**
A description that shows extra information for the player regarding the selected gameover type
*/
[BaseContainerProps(), BaseContainerCustomStringTitleField("Gameover type info (CUSTOM)")]
class SCR_GameOverTypeAttributeDynamicDescription : SCR_BaseAttributeDynamicDescription
{
	protected SCR_GameOverScreenConfig m_GameOverScreenConfig;
	
	protected SCR_BaseEditorAttributeFloatStringValues m_comboboxData;
	
	//------------------------------------------------------------------------------------------------
	override void InitDynamicDescription(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		if (!attribute.IsInherited(SCR_GameOverTypeEditorAttribute))
		{
			Print("'SCR_GameOverTypeAttributeDynamicDescription' is not attached to the 'SCR_GameOverTypeEditorAttribute'!", LogLevel.ERROR);
			return;
		}
	
		SCR_DropdownEditorAttributeUIComponent dropDownUI = SCR_DropdownEditorAttributeUIComponent.Cast(attributeUi);
		if (!dropDownUI)
		{
			Print("'SCR_GameOverTypeAttributeDynamicDescription' is used for 'SCR_GameOverTypeEditorAttribute' but attribute UI does not inherent from SCR_DropdownEditorAttributeUIComponent!", LogLevel.ERROR);
			return;
		}
			
		m_comboboxData = dropDownUI.GetComboBoxData();
		if (!m_comboboxData)
		{
			Print("'SCR_GameOverTypeAttributeDynamicDescription' is used for 'SCR_GameOverTypeEditorAttribute' but could not find m_comboboxData!", LogLevel.ERROR);
			return;
		}
	
		if (!m_GameOverScreenConfig)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
				return;
	
			SCR_GameOverScreenManagerComponent gameOverManager = SCR_GameOverScreenManagerComponent.Cast(gameMode.FindComponent(SCR_GameOverScreenManagerComponent));
			if (!gameOverManager)
				return;
			
			m_GameOverScreenConfig = gameOverManager.GetGameOverConfig();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		if (!super.IsValid(attribute, attributeUi) || !attribute.IsInherited(SCR_GameOverTypeEditorAttribute))
			return false;
		
		return m_comboboxData && m_GameOverScreenConfig && m_DescriptionDisplayInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	override void GetDescriptionData(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi, out SCR_EditorAttributeUIInfo uiInfo, out string param1 = string.Empty, out string param2 = string.Empty, out string param3 = string.Empty)
	{
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
				
		//~ Get info
		int victoryScreenId = m_comboboxData.GetEntryFloatValue(var.GetInt());
		SCR_BaseGameOverScreenInfo gameOverInfo = m_GameOverScreenConfig.GetGameOverInfo(victoryScreenId);
			
		//~ No valid description
		if (!gameOverInfo || gameOverInfo.GetEditorOptionalParams().m_sDescription == string.Empty)
		{
			uiInfo = attribute.GetUIInfo();
			return;
		}
		
		uiInfo = new SCR_EditorAttributeUIInfo();
			
		//~ Copy visuals and overwrite description
		uiInfo.CopyFromEditorAttributeUIInfo(m_DescriptionDisplayInfo, gameOverInfo.GetEditorOptionalParams().m_sDescription);
		
		//~ Has valid description so make sure it is set correctly
		param1 = gameOverInfo.GetEditorOptionalParams().m_sDescriptionParam1;
		param2 = gameOverInfo.GetEditorOptionalParams().m_sDescriptionParam2;		
	}
};
