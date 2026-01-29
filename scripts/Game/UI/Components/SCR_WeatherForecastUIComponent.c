class SCR_WeatherForecastUIComponent: ScriptedWidgetComponent
{	
	[Attribute("Current_Icon")]
	protected string m_sCurrentWeatherIconName;
	
	[Attribute("Current_Name")] 
	protected string m_sCurrentWeatherTextName;
	
	[Attribute("Forecast_Icon")]
	protected string m_sNextWeatherIconName;
	
	[Attribute("Forecast_Title")] 
	protected string m_sNextWeatherTimeName;
	
	[Attribute("Forecast_Name")] 
	protected string m_sNextWeatherTextName;
	
	[Attribute("GameInfo_Weather_Forecast")] 
	protected string m_sNextWeatherHolderName;
	
	[Attribute("#AR-Weather_Forecast_Looping", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sLoopingWeatherText;
	
	[Attribute("1", "Update freq of weather UI in seconds")]
	protected float m_fUIUpdateFreq;
	
	[Attribute("MISSING NAME", desc: "Text shown if weather State has no weather name assigned")]
	protected LocalizedString m_sUnknownWeatherName;
	
	[Attribute("{4B4B51FACB828BF9}UI/Textures/Tasks/TaskIcons/96/Icon_Task_Unknown.edds", desc: "Icon used when weather is unknown")]
	protected ResourceName m_sUnknownWeatherIcon;
	
	protected bool m_bListeningToUpdate;
	
	protected TimeAndWeatherManagerEntity m_TimeAndWeatherEntity;
	protected BaseWeatherStateTransitionManager m_WeatherStateManager;
	protected ref array<ref WeatherState> m_WeatherStates = new array<ref WeatherState>;
	
	protected ImageWidget m_wCurrentWeatherIcon;
	protected TextWidget m_wCurrentWeatherText;
	protected TextWidget m_wNextWeatherText;
	protected TextWidget m_wNextWeatherTimeText;
	protected ImageWidget m_wNextWeatherIcon;
	protected Widget m_wNextWeatherHolder;
	protected Widget m_wRoot;
	
	//~Todo: Update using TimeAndWeather callback once implemented
	protected void CheckWeatherUpdate()
	{	
		if (m_WeatherStateManager.IsPreviewingState())
			return;
		
		WeatherState currentWeather = m_WeatherStateManager.GetCurrentState();		

		//~ Get weather name
		string weatherName = currentWeather.GetLocalizedName();
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(weatherName))
			weatherName = m_sUnknownWeatherName;
			
		//~ Get weather Icon
		string weatherIcon = currentWeather.GetIconPath();
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(weatherIcon))
			weatherIcon = m_sUnknownWeatherIcon;
		
		m_wCurrentWeatherText.SetText(weatherName);
		m_wCurrentWeatherIcon.LoadImageTexture(0, weatherIcon);
		
		WeatherState nextWeather = m_WeatherStateManager.GetNextState();
		if (!nextWeather)
		{
			m_wNextWeatherHolder.SetVisible(false);
			return;
		}
		else 
		{
			m_wNextWeatherHolder.SetVisible(true);
		}
		

		//~ Looping weather or Time progression disabled
		if (m_TimeAndWeatherEntity.IsWeatherLooping() || !m_TimeAndWeatherEntity.GetIsDayAutoAdvanced())
		{
			m_wNextWeatherTimeText.SetText(m_sLoopingWeatherText);
		}
		else 
		{
			float duration = m_WeatherStateManager.GetTimeLeftUntilNextState();
			float changeTime = duration + m_TimeAndWeatherEntity.GetTimeOfTheDay();
			while (changeTime >= 24)
				changeTime -= 24;
			int nextWeatherHour = Math.Floor(changeTime);
			int nextWeatherMinutes = (changeTime - nextWeatherHour) * 60;
			
			if (nextWeatherMinutes >= 0 && nextWeatherMinutes <= 15)
				nextWeatherMinutes = 15;
			else if (nextWeatherMinutes > 15 && nextWeatherMinutes <= 30)
				nextWeatherMinutes = 30;
			else if (nextWeatherMinutes > 30 && nextWeatherMinutes <= 45)
				nextWeatherMinutes = 45;
			else if (nextWeatherMinutes > 45)
			{
				nextWeatherMinutes = 0;
				nextWeatherHour++;
				
				if (nextWeatherHour >= 24)
					nextWeatherHour -= 24;
			}
			
			m_wNextWeatherTimeText.SetTextFormat(SCR_FormatHelper.GetTimeFormattingHoursMinutes(nextWeatherHour, nextWeatherMinutes));
		}
		
		weatherName = nextWeather.GetLocalizedName();
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(weatherName))
			weatherName = m_sUnknownWeatherName;
			
		
		weatherIcon = nextWeather.GetIconPath();
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(weatherIcon))
			weatherIcon = m_sUnknownWeatherIcon;

		m_wNextWeatherText.SetText(weatherName); 
		m_wNextWeatherIcon.LoadImageTexture(0, weatherIcon);
	}
	
	protected void ShowWeatherUI(bool show)
	{
		m_wRoot.SetVisible(show);
	}
	
	
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		m_wRoot = w;
		m_wCurrentWeatherIcon = ImageWidget.Cast(w.FindAnyWidget(m_sCurrentWeatherIconName));
		m_wCurrentWeatherText = TextWidget.Cast(w.FindAnyWidget(m_sCurrentWeatherTextName));
		
		if (!m_wCurrentWeatherIcon || !m_wCurrentWeatherText)
		{
			Print("'SCR_WeatherForecastUIComponent' is missing current weather widgets and won't work.", LogLevel.WARNING);
			return;
		}
		
		m_wNextWeatherIcon  = ImageWidget.Cast(w.FindAnyWidget(m_sNextWeatherIconName));
		m_wNextWeatherText = TextWidget.Cast(w.FindAnyWidget(m_sNextWeatherTextName));
		m_wNextWeatherTimeText = TextWidget.Cast(w.FindAnyWidget(m_sNextWeatherTimeName));
		
		if (!m_wNextWeatherIcon || !m_wNextWeatherText || !m_wNextWeatherTimeText)
		{
			Print("'SCR_WeatherForecastUIComponent' is missing next weather widgets and won't work.", LogLevel.WARNING);
			return;
		}
		
		m_wNextWeatherHolder = w.FindAnyWidget(m_sNextWeatherHolderName);	
		
		ChimeraWorld world = GetGame().GetWorld();	
		m_TimeAndWeatherEntity = world.GetTimeAndWeatherManager();
		
		if (!m_TimeAndWeatherEntity)
		{
			ShowWeatherUI(false);
			return;
		}
			
		
		m_TimeAndWeatherEntity.GetWeatherStatesList(m_WeatherStates);
		if (m_WeatherStates.IsEmpty())
		{
			ShowWeatherUI(false);
			return;
		}
		
		m_WeatherStateManager = m_TimeAndWeatherEntity.GetTransitionManager();
		
		if (!m_WeatherStateManager)
		{
			ShowWeatherUI(false);
			return;
		}
		
		CheckWeatherUpdate();
		m_bListeningToUpdate = true;
		GetGame().GetCallqueue().CallLater(CheckWeatherUpdate, m_fUIUpdateFreq * 1000, true);
	}
	override void HandlerDeattached(Widget w)
	{
		if (m_bListeningToUpdate)
			GetGame().GetCallqueue().Remove(CheckWeatherUpdate);
	}
};
