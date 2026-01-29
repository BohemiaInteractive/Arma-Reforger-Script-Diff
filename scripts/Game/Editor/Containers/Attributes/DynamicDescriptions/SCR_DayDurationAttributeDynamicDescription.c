/**
Dynamic description for daytime duration to display to compare the ingame time with realife time
*/
[BaseContainerProps(), BaseContainerCustomStringTitleField("Day description (CUSTOM)")]
class SCR_DayDurationAttributeDynamicDescription : SCR_BaseAttributeDynamicDescription
{
	//------------------------------------------------------------------------------------------------
	override void InitDynamicDescription(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		super.InitDynamicDescription(attribute);
		
		if (!attribute.IsInherited(SCR_DayDurationEditorAttribute))
			Print("'SCR_DayDurationAttributeDynamicDescription' is not attached to the 'SCR_DayDurationEditorAttribute'!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		if (!super.IsValid(attribute, attributeUi) || !attribute.IsInherited(SCR_DayDurationEditorAttribute))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void GetDescriptionData(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi, out SCR_EditorAttributeUIInfo uiInfo, out string param1 = string.Empty, out string param2 = string.Empty, out string param3 = string.Empty)
	{
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
		
		//Calculate how many real life seconds 1 in game hour is
		float realLifeSecondsFloat = Math.Round(3600 / var.GetFloat());
		
		int days, hours, minutes, seconds;
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds((int)realLifeSecondsFloat, days, hours, minutes, seconds);
		
		param1 = hours.ToString();
		param2 = minutes.ToString();
		param3 = seconds.ToString();
		uiInfo = m_DescriptionDisplayInfo;
	}
};
