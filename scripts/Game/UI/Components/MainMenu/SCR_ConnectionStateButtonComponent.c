//! Component for core menu header button that shows connection state based on comm test, but also needs to show a warning for low bandwidth
//! Uses the parent's icon & label attributes for Low Bandwidth state and considers it the default, as it is outside of the comm test results
class SCR_ConnectionStateButtonComponent : SCR_CoreMenuHeaderButtonComponent
{
	// services
	[Attribute()]
	protected ref SCR_BackendServiceDisplayPresets m_ServicesPresets;
	
	// button states
	[Attribute()]
	protected ref array<ref SCR_ConnectionStateButtonComponent_StatusPreset> m_aStatuses;
	
	// Low Bandwidth
	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_ACTIVE_STANDBY), UIWidgets.ColorPicker)]
	protected ref Color m_LowBandwidthColor;
	
	[Attribute("1")]
	protected bool m_bShowLowBandwidthLabel;
	
	[Attribute("ConnectionState")]
	protected string m_sTooltipTag;
	
	[Attribute(UIConstants.PROCESSING_SPINNER_ANIMATION_SPEED.ToString())]
	protected float m_fSpinnerSpeed;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(EAnimationCurve.EASE_IN_OUT_SINE), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EAnimationCurve))]
	EAnimationCurve m_eSpinnerAnimationCurve;
	
	protected bool m_bIsListeningForCommCheckEvent;
	
	protected ref array<string> m_aUnavailableServices = {};
	protected SCR_ConnectionStateButtonComponent_StatusPreset m_CurrentStatusPreset;
	protected SCR_ScriptedWidgetTooltip m_Tooltip;

	protected ImageWidget m_wConnectingSpinner;
	protected WidgetAnimationImageRotation m_SpinnerAnimation;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (GetGame().InPlayMode())
		{
			SetVisible(false);
			UpdateStatus(SCR_ServicesStatusHelper.GetLastReceivedCommStatus());
		}
		
		m_wConnectingSpinner = ImageWidget.Cast(w.FindAnyWidget("ConnectingSpinner"));
		SCR_ConnectionStateButtonComponent_StatusPreset connectingPreset = GetStatusPreset(SCR_ECommStatus.RUNNING);
		if (m_wConnectingSpinner && connectingPreset)
			m_wConnectingSpinner.SetColor(connectingPreset.m_Color);

		// Owner menu events
		SCR_MenuHelper.GetOnMenuShow().Insert(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuFocusGained().Insert(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuHide().Insert(OnMenuDisabled);
		
		GetButton().m_OnClicked.Insert(OnButtonClicked);
		GetButton().m_OnFocus.Insert(OnButtonFocused);
		GetButton().m_OnFocusLost.Insert(OnButtonFocusLost);
		
		// Automatic Refresh
		GetGame().GetCallqueue().CallLater(OnAutomaticRefresh, SCR_ServicesStatusHelper.AUTOMATIC_REFRESH_RATE, true);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		ClearInvokers();
		
		// Owner menu events
		SCR_MenuHelper.GetOnMenuShow().Remove(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuFocusGained().Remove(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuHide().Remove(OnMenuDisabled);
		
		// Automatic Refresh
		GetGame().GetCallqueue().Remove(OnAutomaticRefresh);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetVisible(bool visible, bool animate = false, float rate = UIConstants.FADE_RATE_FAST)
	{
		super.SetVisible(visible, animate, rate);
		
		if (!visible && m_Tooltip)
			m_Tooltip.ForceHidden();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuEnabled(ChimeraMenuBase menu)
	{
		if (menu != ChimeraMenuBase.GetOwnerMenu(GetRootWidget()))
			return;
		
		if (!m_bIsListeningForCommCheckEvent)
		{
			SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusUpdated);
			SCR_ServicesStatusHelper.GetOnCommStatusCheckStart().Insert(OnCommStatusCheckStart);
		}
		
		m_bIsListeningForCommCheckEvent = true;
		
		SCR_ServicesStatusHelper.RefreshPing();
		
		UpdateStatus(SCR_ServicesStatusHelper.GetLastReceivedCommStatus());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuDisabled(ChimeraMenuBase menu)
	{
		if (menu != ChimeraMenuBase.GetOwnerMenu(GetRootWidget()))
			return;
		
		ClearInvokers();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearInvokers()
	{
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusUpdated);
		SCR_ServicesStatusHelper.GetOnCommStatusCheckStart().Remove(OnCommStatusCheckStart);
		m_bIsListeningForCommCheckEvent = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusUpdated(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateStatus(status);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckStart()
	{
		if (m_CurrentStatusPreset && m_CurrentStatusPreset.m_Status != SCR_ServicesStatusHelper.GetLastReceivedCommStatus())
			UpdateStatus(SCR_ServicesStatusHelper.GetLastReceivedCommStatus());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAutomaticRefresh()
	{
		SCR_ServicesStatusHelper.RefreshPing();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateStatus(SCR_ECommStatus status)
	{
		switch(status)
		{
			case SCR_ECommStatus.FINISHED: 
			{
				if (AreServicesUnavailable())
				{
					UpdateWidgets(status);
					break;
				}
				
				// everything ok, check for low bandwidth
				if (CheckLowBandwidth())
				{
					UpdateWidgets(status, true);
					break;
				}
				
				// no issues, no need to show the button
				SetVisible(false, IsVisible());
				break;
			}
			
			default:
			{
				UpdateWidgets(status);
			 	break;
			}
		}
		
		AnimateSpinner();
		UpdateTooltip();
	}
	
	//------------------------------------------------------------------------------------------------
	// \return true in case services are unavailable and caches the problematic ones to be displayed in tooltips
	protected bool AreServicesUnavailable()
	{
		if (!m_ServicesPresets)
			return true;
		
		m_aUnavailableServices.Clear();

		// Check states of services
		foreach (SCR_BackendServiceDisplay serviceInfo : m_ServicesPresets.GetServices())
		{
			if (!SCR_ServicesStatusHelper.DisplayServiceOnCurrentPlatform(serviceInfo))
				continue;
			
			ServiceStatusItem statusItem = SCR_ServicesStatusHelper.GetStatusByName(serviceInfo.m_sServiceId);
			
			if (!statusItem || statusItem.Status() != SCR_ServicesStatusHelper.STATUS_OK)
				m_aUnavailableServices.Insert(serviceInfo.m_sTitle);
		}
		
		// Main status check
		ServiceStatusItem mainStatus = SCR_ServicesStatusHelper.GetMainStatus();
		return !mainStatus || mainStatus.Status().IsEmpty() || !m_aUnavailableServices.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected bool CheckLowBandwidth()
	{
		//TODO: update this if we ever get an API for bandwidth
		int ping = SCR_ServicesStatusHelper.GetPingValue();
		return ping > SCR_ServicesStatusHelper.PING_THRESHOLD_AWFUL || ping < 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateWidgets(SCR_ECommStatus status, bool lowBandwidth = false)
	{
		m_CurrentStatusPreset = GetStatusPreset(status);		
		SetVisible(m_CurrentStatusPreset != null, true);
		
		if (!m_CurrentStatusPreset)
			return;

		string icon = m_CurrentStatusPreset.m_sIcon;
		Color color = m_CurrentStatusPreset.m_Color;
		string label = m_CurrentStatusPreset.m_sLabel;
		bool showLabel = m_CurrentStatusPreset.m_bShowLabel;
		
		if (lowBandwidth)
		{
			icon = m_sIconName;
			color = m_LowBandwidthColor;
			label = m_sLabel;
			showLabel = m_bShowLowBandwidthLabel;
		}
		
		SetIcon(icon);
		SetIconColor(color);
		
		if (showLabel)
			SetLabelText(label);
		else
			SetLabelText(string.Empty);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AnimateSpinner()
	{
		if (m_SpinnerAnimation)
		{
			m_SpinnerAnimation.GetOnCycleCompleted().Remove(AnimateSpinner);
			if (!ShowSpinner())
				m_SpinnerAnimation.Stop();
		}

		if (!m_wConnectingSpinner)
			return;

		m_wConnectingSpinner.SetVisible(ShowSpinner());

		m_wConnectingSpinner.SetRotation(0);
		m_SpinnerAnimation = AnimateWidget.Rotation(m_wConnectingSpinner, 360, m_fSpinnerSpeed);
		if (m_SpinnerAnimation)
		{
			m_SpinnerAnimation.SetRepeat(true);
			m_SpinnerAnimation.SetCurve(m_eSpinnerAnimationCurve);
			m_SpinnerAnimation.GetOnCycleCompleted().Insert(AnimateSpinner);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ShowSpinner()
	{
		return m_CurrentStatusPreset && m_CurrentStatusPreset.m_Status == SCR_ECommStatus.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ConnectionStateButtonComponent_StatusPreset GetStatusPreset(SCR_ECommStatus status)
	{
		foreach (SCR_ConnectionStateButtonComponent_StatusPreset preset : m_aStatuses)
		{
			if (preset.m_Status == status)
				return preset;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetIconColor(Color color)
	{
		if (!m_wIcon)
			return;
		
		m_wIcon.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonClicked()
	{
		SCR_CommonDialogs.CreateServicesStatusDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonFocused()
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonFocusLost()
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		if (tooltip.GetTag() != m_sTooltipTag)
		{
			m_Tooltip = null;
			return;
		}

		m_Tooltip = tooltip;
		UpdateTooltip();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateTooltip()
	{
		if (!m_Tooltip || !m_CurrentStatusPreset)
			return;

		string message = m_CurrentStatusPreset.m_sTooltipMessage;
		Color messageColor = m_CurrentStatusPreset.m_Color;
		
		SCR_ScriptedWidgetTooltipContentBase content = m_Tooltip.GetContent();
		if (!content)
			return;
		
		if (m_CurrentStatusPreset.m_Status == SCR_ECommStatus.FINISHED)
		{
			// Unavailable services: the tooltips will display the list of issues
			if (!m_aUnavailableServices.IsEmpty())
			{
				// fill the tooltip with the list of issues
				Widget tooltipContent = content.GetContentRoot();
				if (tooltipContent)
				{
					SCR_ListTooltipComponent comp = SCR_ListTooltipComponent.FindComponent(tooltipContent);
					if (comp)
						comp.Init(m_aUnavailableServices);
				}
			}
			
			// Low bandwidth: this is the default message set in the tooltip's .conf file
			else if (CheckLowBandwidth())
			{
				message = content.GetDefaultMessage();
				messageColor = m_LowBandwidthColor;
			}
			
			// if everything is of the button will disappear and so should the tooltip
			else
			{
				m_Tooltip.ForceHidden();
			}
		}
		
		content.SetMessage(message);
		content.SetMessageColor(messageColor);
	}
}

[BaseContainerProps()]
class SCR_ConnectionStateButtonComponent_StatusPreset
{
	[Attribute(defvalue: UIConstants.ICONS_IMAGE_SET, params: "imageset")]
	ResourceName m_sImageSet;

	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_ECommStatus.RUNNING), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECommStatus))]
	SCR_ECommStatus m_Status;

	[Attribute(SCR_ConnectionUICommon.ICON_CONNECTION)]
	string m_sIcon;
	
	[Attribute(SCR_ConnectionUICommon.MESSAGE_CONNECTING)]
	string m_sLabel;

	[Attribute(UIColors.GetColorAttribute(UIColors.CONTRAST_COLOR), UIWidgets.ColorPicker)]
	ref Color m_Color;
	
	[Attribute(SCR_ConnectionUICommon.MESSAGE_CONNECTING)]
	string m_sTooltipMessage;
	
	[Attribute("1")]
	bool m_bShowLabel;
}
