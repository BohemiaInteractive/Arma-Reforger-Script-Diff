//#define DEBUG_STATS_PANELS

enum EStatsPanelState
{
	INIT,
	DEFAULT,
	WARNING,
	ERROR
};

enum EStatsPanelEval
{
	SHOW_AVERAGE,
	SHOW_HIGHEST,
	SHOW_LOWEST
};

class SCR_StatsPanelBase : SCR_InfoDisplayExtended
{
	[Attribute("#AR-ValueUnit_Milliseconds", UIWidgets.EditBox, "Pattern defining how the value and units are displayed (e.g. '%1 ms'.")]
	protected string m_sFormattingPattern;

	[Attribute("50", UIWidgets.SpinBox, "Threshold value for 'warning visual' style. Set it to same or higher value than 'm_iValueError' to disable 'warning visual' style.")]
	protected int m_iValueWarning;	

	[Attribute("100", UIWidgets.SpinBox, "Threshold value for 'error visual' style.")]
	protected int m_iValueError;
	
	[Attribute("999", UIWidgets.SpinBox, "Max. value that can be displayed in the widget. Used to cap the value and also to reserve the space for the value text, to prevent visual glitches.")]
	protected int m_iValueMax;	

	[Attribute("ping-high", UIWidgets.EditBox, "OK icon, used when value is bellow 'm_iValueWarning' threshold.")]
	protected string m_sIconOK;	
	
	[Attribute("ping-high", UIWidgets.EditBox, "Warning icon, used when value is above 'm_iValueWarning', but under 'm_iValueError' threshold.")]
	protected string m_sIconWarning;
	
	[Attribute("ping-high", UIWidgets.EditBox, "Error icon, used when value is above 'm_iValueError' threshold.")]
	protected string m_sIconError;

	[Attribute("", UIWidgets.EditBox, "Label displayed next to the icon (optional).")]
	protected string m_sLabel;		
			
	[Attribute("1", UIWidgets.SpinBox, "How often the display is updated (in seconds).", "1 10")]
	protected int m_iUpdateInterval;	

	[Attribute(SCR_Enum.GetDefault(EStatsPanelEval.SHOW_AVERAGE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EStatsPanelEval))]
	EStatsPanelEval m_eValueCalc;	
	
	[Attribute("0", UIWidgets.CheckBox, "Should the indicator be shown in single player game?")]
	protected bool m_bShowInSinglePlayer;		
	
	[Attribute("1", UIWidgets.CheckBox, "Should the indicator be shown on server?")]
	protected bool m_bShowOnServer;		
	
	RplIdentity m_RplIdentity;
	ref SCR_StatsPanelWidgets m_Widgets;
	protected EStatsPanelState m_eState = EStatsPanelState.INIT;
	
	protected bool m_bShowInDefaultState;			// Based on SCR_InfoDisplay 'm_bShowWhenCreated' flag

	protected float m_fTimeElapsed;
	protected int m_iRecords;
	
	protected float m_fValueToShow;
	protected float m_fValueRecorded;
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		if (m_iUpdateInterval <= 0)
			return false;
		
		if (!Replication.IsRunning() && !m_bShowInSinglePlayer)
			return false;
		
		if (Replication.IsRunning() && Replication.IsServer() && !m_bShowOnServer)
			return false;
		
		m_RplIdentity = RplIdentity.Local();
		
		return true;
	}	
		
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		m_Widgets = new SCR_StatsPanelWidgets();
		m_Widgets.Init(m_wRoot);

		m_Widgets.m_wLabel.SetText(m_sLabel);
				
		// Fill-in the max value to reserve space in the stats bar
		string text = WidgetManager.Translate(m_sFormattingPattern, m_iValueMax);
		m_Widgets.m_wTextPlaceholder.SetText(text);
		
		m_bShowInDefaultState = m_bShown;
		
		float value = GetValue();
		Update(value);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_wRoot)
			return;
		
		m_fTimeElapsed += timeSlice;
		
		float value = GetValue();
		
		// Increment records counter
		m_iRecords++;
		
		if (m_eValueCalc == EStatsPanelEval.SHOW_AVERAGE)
		{
			m_fValueRecorded = m_fValueRecorded + value;
		}
		else
		{
			if (m_eValueCalc == EStatsPanelEval.SHOW_HIGHEST && value > m_fValueRecorded) m_fValueRecorded = value;
			else if (m_eValueCalc == EStatsPanelEval.SHOW_LOWEST && value < m_fValueRecorded) m_fValueRecorded = value;
		}
		
		if (m_fTimeElapsed < m_iUpdateInterval)
			return;
		
		// Calculate avg stat value
		if (m_iRecords > 0)
		{
			if (m_eValueCalc == EStatsPanelEval.SHOW_AVERAGE)
				m_fValueToShow = m_fValueRecorded/m_iRecords;
			else
				m_fValueToShow = m_fValueRecorded;
			
			Update(m_fValueToShow);
		}
		
		// Reset after update
		m_iRecords = 0;
		m_fValueRecorded = 0;
		m_fTimeElapsed = m_fTimeElapsed - m_iUpdateInterval;		
	}	
	
	//------------------------------------------------------------------------------------------------
	protected float GetValue()
	{
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Update(float value)
	{
		value = Math.Clamp(Math.Round(value), 0, m_iValueMax);		

		/*
		if (this.Type() == SCR_StatsPanel_PacketLoss)
		{
			PrintFormat("Packet loss: %1\%", value)
		}
		*/	
		
		// Update text
		string text = WidgetManager.Translate(m_sFormattingPattern, value);	
		m_Widgets.m_wText.SetText(text);		
			
		// Evaluate the state based on current value	
		EStatsPanelState state = EStatsPanelState.DEFAULT;

		// Determine state for case where lower is better (like latency or packet loss)
		if (m_iValueError >= m_iValueWarning)
		{
			if (value > m_iValueError)
				state = EStatsPanelState.ERROR; 
			else if (value > m_iValueWarning)
				state = EStatsPanelState.WARNING;		
		}
		// Determine state for case where higher is better (like FPS)
		else
		{
			if (value < m_iValueError)
				state = EStatsPanelState.ERROR; 
			else if (value < m_iValueWarning)
				state = EStatsPanelState.WARNING;		
		}
		
		// State didn't change, no need to update visuals (text already updated)
		if (m_eState == state)
			return;

		m_eState = state;
		
		// Get visialization attributes based on the new state
		float opacity;
		Color color;
		Color colorGlow;
		string icon;		
				
		switch (state)
		{
			case EStatsPanelState.WARNING:
				opacity = 1;	
				color = GUIColors.ORANGE_BRIGHT2;
				//colorGlow = GUIColors.ORANGE;
				colorGlow = GUIColors.DEFAULT_GLOW;
				icon	 = m_sIconWarning;
			break;
			
			case EStatsPanelState.ERROR:
				opacity = 1;	
				color = GUIColors.RED_BRIGHT2;
				//colorGlow = GUIColors.RED;
				colorGlow = GUIColors.DEFAULT_GLOW;
				icon	 = m_sIconError;
			break;			
			
			default:
				opacity = 0.5;	
				color = GUIColors.DEFAULT;
				colorGlow = GUIColors.DEFAULT_GLOW;
				icon	 = m_sIconOK;
			break;
		}		
		
		// Set global color & opacity
		m_Widgets.m_wColorOpacity.SetOpacity(opacity);
		m_Widgets.m_wColorOpacity.SetColor(color);

		// Set icon & glow		
		m_Widgets.m_wIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, icon);
		m_Widgets.m_wIconGlow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, icon);
		m_Widgets.m_wIconGlow.SetColor(colorGlow);
		
		// Update text shadow
		m_Widgets.m_wText.SetShadow(20, colorGlow.PackToInt(), 0.35); 
		
		// Set visibility
		Show((state == EStatsPanelState.DEFAULT && m_bShowInDefaultState) || state != EStatsPanelState.DEFAULT);
	}	
}