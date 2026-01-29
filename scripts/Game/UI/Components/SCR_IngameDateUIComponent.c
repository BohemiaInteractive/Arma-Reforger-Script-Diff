class SCR_IngameDateUIComponent : ScriptedWidgetComponent
{
	[Attribute("Date_Date", desc: "Widget filled with date. At least Date or Day widget needs to be assigned for this UI to function")]
	protected string m_sDateTextName;

	[Attribute("Date_Day", desc: "Widget filled with current day (Monday to Sunday). At least Date or Day widget needs to be assigned for this UI to function")]
	protected string m_sDayTextName;
	
	[Attribute("1", desc: "If true month will be written in full text, else month will be a number")]
	protected bool m_bMonthAsText;
	
	[Attribute("2", desc: "Time until the UI will update in seconds")]
	protected float m_fUpdateFreq;
	
	protected int m_iDayLastCheckedYear = -1;
	protected int m_iDayLastCheckedMonth = -1;
	protected int m_iDayLastCheckedDay = -1;
	
	//~ References
	protected TextWidget m_DateText;
	protected TextWidget m_DayText;
	protected TimeAndWeatherManagerEntity m_TimeAndWeatherManagerEntity;
	
	
	protected void UpdateIngameDate()
	{
		int day, month, year;
		m_TimeAndWeatherManagerEntity.GetDate(year, month, day);
		
		//~ Set date
		if (m_DateText)
		{
			string date = SCR_DateTimeHelper.GetDateString(day, month, year, m_bMonthAsText);
			m_DateText.SetText(date);
		}
		
		if (m_DayText)
		{
			if (m_iDayLastCheckedDay != day || m_iDayLastCheckedMonth != month || m_iDayLastCheckedYear != year)
			{
				m_iDayLastCheckedDay = day;
				m_iDayLastCheckedMonth = month;
				m_iDayLastCheckedYear = year;
			
				m_DayText.SetText(m_TimeAndWeatherManagerEntity.GetWeekDayString());
			}
		}
		
	}
	
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		m_DateText = TextWidget.Cast(w.FindAnyWidget(m_sDateTextName));
		m_DayText = TextWidget.Cast(w.FindAnyWidget(m_sDayTextName));
		
		if (!m_DateText && !m_DayText)
		{
			Print("'SCR_IngameDateUIComponent' is missing both date and day widget and therefore won't function", LogLevel.ERROR);
			return;
		}
		
		ChimeraWorld world = GetGame().GetWorld();
		m_TimeAndWeatherManagerEntity = world.GetTimeAndWeatherManager();
		if (!m_TimeAndWeatherManagerEntity)
		{
			Print("'SCR_IngameDateUIComponent' could not find the TimeAndWeatherManager", LogLevel.ERROR);
			return;
		}
		
		UpdateIngameDate();
		GetGame().GetCallqueue().CallLater(UpdateIngameDate, m_fUpdateFreq * 1000, true);
		
	}
	override void HandlerDeattached(Widget w)
	{
		if (m_TimeAndWeatherManagerEntity)
			GetGame().GetCallqueue().Remove(UpdateIngameDate);
	}
};