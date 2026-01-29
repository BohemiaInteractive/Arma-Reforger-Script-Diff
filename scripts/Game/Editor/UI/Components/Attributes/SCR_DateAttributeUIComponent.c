// Script File 
/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_DateAttributeUIComponent: SCR_BaseEditorAttributeUIComponent
{
	[Attribute("ComboRoot")]
	protected string m_sComboBaseName;
	
	[Attribute(defvalue: "DateLabel")]
	protected string m_sLabelHolderBaseName;
	
	[Attribute(defvalue: "Label")]
	protected string m_sLabelName;
	
	[Attribute()]
	protected ref array<int> m_aDayArray;
	
	[Attribute(defvalue: "1.25")]
	protected float m_fMonthWidgetFill;
	
	protected TextWidget m_Label0;
	protected TextWidget m_Label1;
	protected TextWidget m_Label2;
	
	protected SCR_ComboBoxIconComponent m_comboBox0;
	protected SCR_ComboBoxIconComponent m_comboBox1;
	protected SCR_ComboBoxIconComponent m_comboBox2;
	
	protected ref SCR_EditorAttributeEntryStringArray m_aMonthData;
	protected ref SCR_EditorAttributeEntryIntArray m_aYearData;
	
	protected string m_sDateOrder;
	
	protected TimeAndWeatherManagerEntity m_TimeAndWeatherManager;
	
	//---- REFACTOR NOTE START: Hardcoded numbers
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{	
		super.Init(w, attribute);
		
		ChimeraWorld world = GetGame().GetWorld();
		m_TimeAndWeatherManager = world.GetTimeAndWeatherManager();
		
		if (!m_TimeAndWeatherManager)
		{
			Print("SCR_DateAttributeUIComponent could not find GetTimeAndWeatherManager!", LogLevel.ERROR);
			return;
		}
			
		//Date holder
		Widget dateHolderWidget = w.FindAnyWidget(m_sUiComponentName);
		if (!dateHolderWidget) return;
		
	 	SCR_ToolboxComponent dateHolder = SCR_ToolboxComponent.Cast(dateHolderWidget.FindHandler(SCR_ToolboxComponent));
		if (!dateHolder) return;
		
		//Get Labels
		Widget labelHolder0 = w.FindAnyWidget(m_sLabelHolderBaseName + 0);
		Widget labelHolder1 = w.FindAnyWidget(m_sLabelHolderBaseName + 1);
		Widget labelHolder2 = w.FindAnyWidget(m_sLabelHolderBaseName + 2);		
		
		if (!labelHolder0 || !labelHolder1 || !labelHolder2) return;
	
		m_Label0 = TextWidget.Cast(labelHolder0.FindAnyWidget(m_sLabelName));
		m_Label1 = TextWidget.Cast(labelHolder1.FindAnyWidget(m_sLabelName));
		m_Label2 = TextWidget.Cast(labelHolder2.FindAnyWidget(m_sLabelName));
		
		if (!m_Label0 || !m_Label1 || !m_Label2) return;
		
		Widget comboBoxWidget0 = w.FindAnyWidget(m_sComboBaseName + 0);
		Widget comboBoxWidget1 = w.FindAnyWidget(m_sComboBaseName + 1);
		Widget comboBoxWidget2 = w.FindAnyWidget(m_sComboBaseName + 2);
		
		if (!comboBoxWidget0 || !comboBoxWidget1 || !comboBoxWidget2) return;
		
		m_comboBox0 = SCR_ComboBoxIconComponent.Cast(comboBoxWidget0.FindHandler(SCR_ComboBoxIconComponent));
		m_comboBox1 = SCR_ComboBoxIconComponent.Cast(comboBoxWidget1.FindHandler(SCR_ComboBoxIconComponent));
		m_comboBox2 = SCR_ComboBoxIconComponent.Cast(comboBoxWidget2.FindHandler(SCR_ComboBoxIconComponent));
		
		if (!m_comboBox0 || !m_comboBox1 || !m_comboBox2) return;

		//Set daytime presets
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var) return;
		
		array<ref SCR_BaseEditorAttributeEntry> entries = new array<ref SCR_BaseEditorAttributeEntry>;
		int entriesCount = attribute.GetEntries(entries);
		string dayLabel, monthLabel, yearLabel;
		
		if (entriesCount > 1)
		{
			SCR_BaseEditorAttributeEntryText stringData = SCR_BaseEditorAttributeEntryText.Cast(entries[0]);
			if (!stringData) return;
			dateHolder.SetLabel(stringData.GetText());
			
			stringData = SCR_BaseEditorAttributeEntryText.Cast(entries[1]);
			if (!stringData) return;
			m_sDateOrder = stringData.GetText();
			
			stringData = SCR_BaseEditorAttributeEntryText.Cast(entries[2]);
			if (!stringData) return;
			dayLabel = stringData.GetText();
			
			stringData = SCR_BaseEditorAttributeEntryText.Cast(entries[3]);
			if (!stringData) return;
			monthLabel = stringData.GetText();
			
			stringData = SCR_BaseEditorAttributeEntryText.Cast(entries[4]);
			if (!stringData) return;
			yearLabel = stringData.GetText();
			
			m_aMonthData = SCR_EditorAttributeEntryStringArray.Cast(entries[5]);
			if (!m_aMonthData) return;
			
			m_aYearData = SCR_EditorAttributeEntryIntArray.Cast(entries[6]);
			if (!m_aYearData) return;
		}
		
		FillMonthComboBox(GetCorrectComboBox("m"));
		FillYearComboBox(GetCorrectComboBox("y"));
		
		vector date = var.GetVector();
		CreateDayList(GetCorrectComboBox("d"), date[1] +1, m_aYearData.GetEntry(date[2]), false);
		
		//Lables
		GetCorrectLabel("d").SetText(dayLabel);
		GetCorrectLabel("m").SetText(monthLabel);
		GetCorrectLabel("y").SetText(yearLabel);
		
		//Month fill weight
		SetMonthWidgetFillWeight();
		
		//Events
		m_comboBox0.m_OnChanged.Insert(OnComboBoxChanged);
		m_comboBox1.m_OnChanged.Insert(OnComboBoxChanged);
		m_comboBox2.m_OnChanged.Insert(OnComboBoxChanged);
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{
		if (!var) 
			return;
		
		vector date = var.GetVector();
		SetDateComboBoxes(date);
		CreateDayList(GetCorrectComboBox("d"), date[1] +1, m_aYearData.GetEntry(date[2]));
		UpdateDateMoonPhaseIcon();
		
		super.SetFromVar(var);
	}
	
	//---- REFACTOR NOTE END ----
	
	protected void CreateDayList(SCR_ComboBoxIconComponent comboBox, int currentMonth, int currentYear, bool setMoonIcon = true)
	{		
		int currentDateIndex = comboBox.GetCurrentIndex();
		comboBox.ClearAll();
		int count = m_aDayArray.Count();

		if (count == 0) return;
		
		ChimeraWorld world = GetGame().GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager) return;
		
		float daytime;		
		int day, month, year;
		GetCorrectDateTime(daytime, day, month, year);
		float timeZoneOffset = m_TimeAndWeatherManager.GetTimeZoneOffset();
		float dstOffset = m_TimeAndWeatherManager.GetDSTOffset();
		float latitude = m_TimeAndWeatherManager.GetCurrentLatitude();
		
		SCR_MoonPhaseUIInfo moonPhaseInfo;
		ResourceName preIcon;
		ResourceName icon;
		float rotation = 0;
		
		if (latitude < 0)
			rotation = 180;
		
		for (int i = 0; i < count; i++)
		{
			//Check if valid date for the month and year
			if (m_aDayArray[i] >= 29 && !timeManager.CheckValidDate(currentYear, currentMonth, m_aDayArray[i]))
				break;
			
			if (setMoonIcon)
			{
				moonPhaseInfo =  m_TimeAndWeatherManager.GetMoonPhaseInfoForDate(year, month, m_aDayArray[i], daytime, timeZoneOffset, dstOffset, latitude);
				icon = moonPhaseInfo.GetIconPath();
					
				if (preIcon == icon)
					icon = string.Empty;
				else
					preIcon = icon;
			}
			
			comboBox.AddItemAndIcon(m_aDayArray[i].ToString(), icon, rotation);
		}
		
		//Set valid date if prev date not valid
		while (currentDateIndex >= comboBox.GetNumItems() && currentDateIndex > 0){
			currentDateIndex--;
		}
		
		comboBox.SetCurrentItem(currentDateIndex);
	}
	
	protected void UpdateDateMoonPhaseIcon()
	{
		SCR_BaseEditorAttributeVar dayTimeVar;
		if (!m_AttributeManager.GetAttributeVariable(SCR_DaytimeEditorAttribute, dayTimeVar))
			return;
		
		float daytime;		
		int day, month, year;
		GetCorrectDateTime(daytime, day, month, year);
		float timeZoneOffset = m_TimeAndWeatherManager.GetTimeZoneOffset();
		float dstOffset = m_TimeAndWeatherManager.GetDSTOffset();
		float latitude = m_TimeAndWeatherManager.GetCurrentLatitude();
		
		SCR_ComboBoxIconComponent dayComboBox = GetCorrectComboBox("d");
		SCR_MoonPhaseUIInfo moonPhaseInfo =  m_TimeAndWeatherManager.GetMoonPhaseInfoForDate(year, month, day, daytime, timeZoneOffset, dstOffset, latitude);
		ImageWidget icon = dayComboBox.GetIconWidget();
		moonPhaseInfo.SetIconTo(icon);
		icon.SetRotation(moonPhaseInfo.GetMoonphaseImageRotation());
		icon.SetVisible(true);
	}
	
	protected void GetCorrectDateTime(out float daytime, out int day, out int month, out int year)
	{
		SCR_BaseEditorAttributeVar dayTimeVar;
		if (!m_AttributeManager.GetAttributeVariable(SCR_DaytimeEditorAttribute, dayTimeVar))
			return;
		
		if (!m_comboBox0 || !m_comboBox1 || !m_comboBox2)
			return;
		
		daytime = dayTimeVar.GetFloat() / 3600;		
		day = GetCorrectComboBox("d").GetCurrentIndex() +1;
		month = GetCorrectComboBox("m").GetCurrentIndex() +1;
		year = m_aYearData.GetEntry(GetCorrectComboBox("y").GetCurrentIndex());
	}
	
	protected void FillMonthComboBox(SCR_ComboBoxIconComponent comboBox)
	{		
		int count = m_aMonthData.GetCount();
		comboBox.ClearAll();
		
		for (int i = 0; i < count; i++)
		{
			comboBox.AddItem(m_aMonthData.GetEntry(i));
		}
	}
	
	protected void FillYearComboBox(SCR_ComboBoxIconComponent comboBox)
	{		
		int count = m_aYearData.GetCount();
		comboBox.ClearAll();
		
		for (int i = 0; i < count; i++)
		{
			comboBox.AddItem(m_aYearData.GetEntry(i).ToString());
		}
	}
	
	protected void SetMonthWidgetFillWeight()
	{
		LayoutSlot.SetFillWeight((GetCorrectLabel("m").GetParent()), m_fMonthWidgetFill);
		LayoutSlot.SetFillWeight(GetCorrectComboBox("m").GetRootWidget(), m_fMonthWidgetFill);
	}
	
	protected SCR_ComboBoxIconComponent GetCorrectComboBox(string lookfor)
	{
		if (m_sDateOrder[0] == lookfor)
			return m_comboBox0;
		if (m_sDateOrder[1] == lookfor)
			return m_comboBox1;
		else
			return m_comboBox2;
	}
	
	protected TextWidget GetCorrectLabel(string lookfor)
	{
		if (m_sDateOrder[0] == lookfor)
			return m_Label0;
		if (m_sDateOrder[1] == lookfor)
			return m_Label1;
		else
			return m_Label2;
	}
	
	protected void SetDateComboBoxes(vector date)
	{			
		GetCorrectComboBox("d").SetCurrentItem(date[0]);
		GetCorrectComboBox("m").SetCurrentItem(date[1]);
		GetCorrectComboBox("y").SetCurrentItem(date[2]);
	}
	
	override bool OnChangeInternal(Widget w, int x, int y, bool finished)
	{		
		SCR_BaseEditorAttribute attribute = GetAttribute();
		
		if (!attribute) return false;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariable(true);

		vector date = Vector(GetCorrectComboBox("d").GetCurrentIndex(), GetCorrectComboBox("m").GetCurrentIndex(), GetCorrectComboBox("y").GetCurrentIndex());
		var.SetVector(date);
		
		super.OnChangeInternal(w, x, y, finished);
		return false;
	}
	
	protected void OnComboBoxChanged(SCR_ComboBoxComponent comboBox, int index)
	{			
		//Update day
		if (comboBox == GetCorrectComboBox("m") || comboBox == GetCorrectComboBox("y"))
			CreateDayList(GetCorrectComboBox("d"), GetCorrectComboBox("m").GetCurrentIndex() +1, m_aYearData.GetEntry(GetCorrectComboBox("y").GetCurrentIndex()));

		UpdateDateMoonPhaseIcon();
		
		OnChangeInternal(null, 0, 0, false);
	}
	
	//------------------------------------------------------------------------------------------------\
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);
		
		GetGame().GetWorkspace().SetFocusedWidget(m_comboBox0.GetRootWidget());
		return true;
	}
	
	override void HandlerDeattached(Widget w)
	{		
		super.HandlerDeattached(w); 
		
		if (m_comboBox0)
			m_comboBox0.m_OnChanged.Remove(OnComboBoxChanged);
		if (m_comboBox1)
			m_comboBox1.m_OnChanged.Remove(OnComboBoxChanged);
		if (m_comboBox2)
			m_comboBox2.m_OnChanged.Remove(OnComboBoxChanged);
	}
};

