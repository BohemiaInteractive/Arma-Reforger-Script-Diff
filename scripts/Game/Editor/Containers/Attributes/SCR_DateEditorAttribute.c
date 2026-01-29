[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_DateEditorAttribute : SCR_BaseEditorAttribute
{
	//TODO: Needs proper system to know the order of dmy
	[Attribute(defvalue: "dmy", desc: "The order in which days, months and years appear. this could be dmy ymd mdy etc. This is a temporary system until a more permanent solution is added")]
	protected string m_sDateFormatOrder;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox, defvalue: "#AR-Date_Day")]
	protected LocalizedString m_sDayLabel;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox, defvalue: "#AR-Date_Month")]
	protected LocalizedString m_sMonthLabel;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox, defvalue: "#AR-Date_Year")]
	protected LocalizedString m_sYearLabel;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected ref array<ref LocalizedString> m_aMonthList;
	
	[Attribute(defvalue: "1940")]
	protected int m_iYearStartDate;

	[Attribute(defvalue: "1995")]
	protected int m_iYearEndDate;
	
	protected ref array<int> m_aYearArray = {};

	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager)
			return null;
		
		//Get date
		vector date = Vector(timeManager.GetDay() -1, timeManager.GetMonth() -1, GetYearIndex(timeManager.GetYear()));
		return SCR_BaseEditorAttributeVar.CreateVector(date);
	}	
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) return;
		
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager) return;

		CreateYearArray();
		vector date =  var.GetVector();
		int day = date[0] +1;
		int month = date[1] +1;
		int year = m_aYearArray[date[2]];
		
		if (item)
		{
			SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.EDITOR_ATTRIBUTES_DATE_CHANGED, playerID, day, month, year);
		}
		
		timeManager.SetDate(year, month, day);
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (!var || isInit)
			return;
		
		//If date changed set time preset selected false
		manager.SetAttributeSelected(SCR_TimePresetsEditorAttribute, false, -1);
	}
	
	/*!
	Get year array for dynamic description
	\param[out] yearArray Array of years
	*/
	void GetYearArray(notnull out array<int> yearArray)
	{
		CreateYearArray();
		
		foreach(int year : m_aYearArray)
		{
			yearArray.Insert(year);
		}
	}
	
	protected void CreateYearArray()
	{
		if (m_aYearArray.Count() != 0)
			return;
		
		ChimeraWorld world = GetGame().GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager)
			return;
		
		int currentYear = timeManager.GetYear();
		
		//Safty
		if (currentYear < m_iYearStartDate)
			m_iYearStartDate = currentYear;
		if (currentYear > m_iYearEndDate)
			m_iYearEndDate = currentYear;
		if (m_iYearEndDate <= m_iYearStartDate)
			m_iYearEndDate = m_iYearStartDate +1;
		
		//Create year list
		for (int y = m_iYearStartDate; y <= m_iYearEndDate; y++)
		{
			m_aYearArray.Insert(y);
		}
	}
	
	protected int GetYearIndex(int year)
	{
		CreateYearArray();
		
		if (m_aYearArray.Contains(year))
		{
			return m_aYearArray.Find(year);
		}
		else 
		{
			return 0;
		}
	}
	
	/* Returns year by given index, called by SCR_DaytimeEditorAttribute preview
	\param index given index
	\return int year
	*/
	int GetYearByIndex(int index)
	{
		CreateYearArray();
		
		if (index < 0 || index >= m_aYearArray.Count())
			return 1990;
		
		return m_aYearArray[index];
	}
	
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		outEntries.Insert(new SCR_BaseEditorAttributeEntryText(GetUIInfo().GetName()));
		outEntries.Insert(new SCR_BaseEditorAttributeEntryText(m_sDateFormatOrder));
		outEntries.Insert(new SCR_BaseEditorAttributeEntryText(m_sDayLabel));
		outEntries.Insert(new SCR_BaseEditorAttributeEntryText(m_sMonthLabel));
		outEntries.Insert(new SCR_BaseEditorAttributeEntryText(m_sYearLabel));
		outEntries.Insert(new SCR_EditorAttributeEntryStringArray(m_aMonthList));
		
		CreateYearArray();
		outEntries.Insert(new SCR_EditorAttributeEntryIntArray(m_aYearArray));
		return outEntries.Count();
	}
	
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		if (!manager)
			return;
		
		SCR_BaseEditorAttribute timeAttribute = manager.GetAttributeRef(SCR_DaytimeEditorAttribute);
		if (timeAttribute)
			timeAttribute.PreviewVariable(setPreview, manager);
	}
};