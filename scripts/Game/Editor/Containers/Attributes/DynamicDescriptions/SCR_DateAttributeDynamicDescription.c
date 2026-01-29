/**
Dynamic description for SCR_DateEditorAttribute to show sunrise, sunset and moonphase
*/
[BaseContainerProps(), BaseContainerCustomStringTitleField("Date description (CUSTOM)")]
class SCR_DateAttributeDynamicDescription : SCR_BaseAttributeDynamicDescription
{	
	[Attribute("#AR-Editor_Attribute_Date_Description_NoSunSetOrRise", desc: "Text shown in description when there is no sunset nor sunrise")]
	protected string m_sNoSunriseSunSet;
	
	protected TimeAndWeatherManagerEntity m_TimeAndWeatherManager;
	protected SCR_AttributesManagerEditorComponent m_AttributeManager;
	
	//------------------------------------------------------------------------------------------------
	override void InitDynamicDescription(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		super.InitDynamicDescription(attribute);
		
		if (!m_TimeAndWeatherManager)
		{
			ChimeraWorld world = GetGame().GetWorld();
			m_TimeAndWeatherManager = world.GetTimeAndWeatherManager();
		}
		
		if (!m_AttributeManager)
			m_AttributeManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));

		if (!attribute.IsInherited(SCR_DateEditorAttribute))
			Print("'SCR_DateAttributeDynamicDescription' is not attached to the 'SCR_DateEditorAttribute'!", LogLevel.ERROR);			
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		if (!super.IsValid(attribute, attributeUi) || !attribute.IsInherited(SCR_DateEditorAttribute))
			return false;
		
		return m_TimeAndWeatherManager != null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void GetDescriptionData(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi, out SCR_EditorAttributeUIInfo uiInfo, out string param1 = string.Empty, out string param2 = string.Empty, out string param3 = string.Empty)
	{
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
		
		SCR_DateEditorAttribute dateAttribute = SCR_DateEditorAttribute.Cast(attribute);
		if (!dateAttribute)
			return;
		
		float daytime = 0;
		
		//~ If has daytime var
		SCR_BaseEditorAttributeVar dayTimeVar;
		if (m_AttributeManager && m_AttributeManager.GetAttributeVariable(SCR_DaytimeEditorAttribute, dayTimeVar))
			daytime = dayTimeVar.GetFloat() / 3600;	
		
		array<int> yearArray = {};
		dateAttribute.GetYearArray(yearArray);
		
		vector date = var.GetVector();
		
		int day = date[0] +1;
		int month = date[1] +1;		
		int year = yearArray[date[2]];
		
		float timeZoneOffset = m_TimeAndWeatherManager.GetTimeZoneOffset();
		float dstOffset = m_TimeAndWeatherManager.GetDSTOffset();
		float latitude = m_TimeAndWeatherManager.GetCurrentLatitude();
		
		SCR_MoonPhaseUIInfo moonPhaseInfo =  m_TimeAndWeatherManager.GetMoonPhaseInfoForDate(year, month, day, daytime, timeZoneOffset, dstOffset, latitude);
		
		float sunRiseTime, sunSetTime;
		bool hasSunRise = m_TimeAndWeatherManager.GetSunriseHourForDate(year, month, day, m_TimeAndWeatherManager.GetCurrentLatitude(), m_TimeAndWeatherManager.GetCurrentLongitude(), m_TimeAndWeatherManager.GetTimeZoneOffset(),  m_TimeAndWeatherManager.GetDSTOffset(),  sunRiseTime);
		bool hasSunSet = m_TimeAndWeatherManager.GetSunsetHourForDate(year, month, day, m_TimeAndWeatherManager.GetCurrentLatitude(), m_TimeAndWeatherManager.GetCurrentLongitude(), m_TimeAndWeatherManager.GetTimeZoneOffset(),  m_TimeAndWeatherManager.GetDSTOffset(),  sunSetTime);
		
		string sunRiseName, sunSetName;
		
		if (!hasSunRise)
			sunRiseName = m_sNoSunriseSunSet;
		else 
			sunRiseName = SCR_FormatHelper.GetTimeFormattingHideSeconds(sunRiseTime * 3600, ETimeFormatParam.DAYS);
		if (!hasSunRise)
			sunSetName = m_sNoSunriseSunSet;
		else 
			sunSetName = SCR_FormatHelper.GetTimeFormattingHideSeconds(sunSetTime * 3600, ETimeFormatParam.DAYS);
		
		uiInfo = m_DescriptionDisplayInfo;
		param1 = sunRiseName;
		param2 = sunSetName;
		param3 = moonPhaseInfo.GetName();	
	}
};
