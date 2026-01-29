class SCR_ServicesStatusDialogComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected ref SCR_BackendServiceDisplayPresets m_ServicesPresets;

	[Attribute()]
	protected ref array<ref SCR_ServicesStatusDialogComponent_Status> m_aStatuses;

	[Attribute()]
	protected ref array<ref SCR_ServicesStatusDialogComponent_Status> m_aMainStatuses;
	
	[Attribute(defvalue: "{D6EA742398E63066}UI/layouts/Menus/Dialogs/ServiceStatusLine.layout", params: "layout")]
	protected ResourceName m_LineLayout;
	
	// Ping
	[Attribute(defvalue: " - ")]
	protected string m_sNoPing;

	[Attribute(defvalue: " ... ")]
	protected string m_sUpdatingPing;

	[Attribute(defvalue: "%1+", desc: "Can use %1 for Max Ping display (default \"%1+\" e.g \"999+\")")]
	protected string m_sBigPing;

	// Status message
	[Attribute(defvalue: "#AR-Account_LoginTimeout")]
	protected string m_sStatusesMessageInitializationError;
	
	[Attribute(defvalue: "#AR-ServicesStatus_Message_AttemptingToConnect")]
	protected string m_sStatusesMessageAttemptingToConnect;
	
	[Attribute(defvalue: "#AR-Workshop_Dialog_NoConnection_CheckConnection")]
	protected string m_sStatusesMessageNoConnection;
	
	[Attribute(defvalue: "#AR-ServicesStatus_Message_AllServicesUp")]
	protected string m_sStatusesMessageRunning;
	
	[Attribute(defvalue: "#AR-ServicesStatus_Message_SomeServicesUp")]
	protected string m_sStatusesMessageWarning;
	
	[Attribute(defvalue: "#AR-ServicesStatus_Message_AllServicesDown")]
	protected string m_sStatusesMessageError;
	
	// Last Update message
	[Attribute(defvalue: "#AR-ServicesStatus_LastUpdate_Seconds_Condensed")]
	protected string m_sLastUpdateMessageSeconds;
	
	[Attribute(defvalue: "#AR-ServicesStatus_LastUpdate_Condensed")]
	protected string m_sLastUpdateMessageMinutes;
	
	[Attribute(defvalue: "#AR-Account_LoginTimeout")]
	protected string m_sLastUpdateMessageNoConnection;
	
	// Legend Tooltip
	[Attribute(defvalue: "ServiceStatusLegend")]
	protected string m_sTooltipTag;
	
	[Attribute(defvalue: "#AR-ServicesStatus_Legend_Warning")]
	protected string m_sLegendWarning;
	
	[Attribute(defvalue: "#AR-ServicesStatus_Legend_Running")]
	protected string m_sLegendRunning;
	
	[Attribute(defvalue: "#AR-ServicesStatus_Legend_Error")]
	protected string m_sLegendError;
	
	// Elements
	protected TextWidget m_wPingWidget;
	protected Widget m_wLinesParentWidget;
	protected TextWidget m_wServicesMessage;
	protected TextWidget m_wLastUpdate;
	protected TextWidget m_wRefreshMessage;

	protected string m_sPingText;
	protected string m_sRefreshText;
	
	protected SCR_ScriptedWidgetTooltip m_Tooltip;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!m_LineLayout || !m_ServicesPresets || m_ServicesPresets.GetServices().IsEmpty())
			return;

		// services message
		m_wServicesMessage = TextWidget.Cast(w.FindAnyWidget("ServicesMessage"));
		UpdateServicesMessage(SCR_ServicesStatusHelper.GetLastReceivedCommStatus(), EServiceStatus.ERROR);
		
		// ping
		m_wPingWidget = TextWidget.Cast(SCR_WidgetHelper.GetWidgetOrChild(w, "Ping"));
		if (m_wPingWidget)
		{
			m_sPingText = m_wPingWidget.GetText();
			m_wPingWidget.SetColor(Color.FromInt(UIColors.WARNING.PackToInt()));
		}

		// lines
		m_wLinesParentWidget = SCR_WidgetHelper.GetWidgetOrChild(w, "Statuses");
		if (!m_wLinesParentWidget)
			return;

		// last update
		m_wLastUpdate = TextWidget.Cast(w.FindAnyWidget("LastUpdate"));
		SetLastUpdateMessage(-1);
		
		// refresh message
		m_wRefreshMessage = TextWidget.Cast(w.FindAnyWidget("RefreshCountdown"));
		if (m_wRefreshMessage)
		{
			m_sRefreshText = m_wRefreshMessage.GetText();
			SetRefreshMessage(0, false);
		}
		
		// build info
		ArmaReforgerScripted game = GetGame();
		TextWidget buildInfo = TextWidget.Cast(w.FindAnyWidget("BuildInfo"));
		if (buildInfo)
			buildInfo.SetTextFormat(buildInfo.GetText(), game.GetBuildVersion(), game.GetBuildTime());
		
		CreateLines(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateLines(Widget w)
	{
		if (!m_ServicesPresets)
			return;
		
		SCR_ServicesStatusDialogComponent_Status status;
		if (m_aStatuses && !m_aStatuses.IsEmpty())
			status = m_aStatuses[0];

		Widget line;
		TextWidget titleWidget;
		foreach (SCR_BackendServiceDisplay service : m_ServicesPresets.GetServices())
		{
			if (!SCR_ServicesStatusHelper.DisplayServiceOnCurrentPlatform(service))
				continue;
			
			line = GetGame().GetWorkspace().CreateWidgets(m_LineLayout, m_wLinesParentWidget);
			if (!line)
				continue;

			line.SetName(service.m_sId);
			
			SCR_ServicesStatusDialogLineComponent lineComp = SCR_ServicesStatusDialogLineComponent.FindComponent(line);
			if (!lineComp)
				continue;
			
			if (lineComp.m_wTitle)
				lineComp.m_wTitle.SetText(service.m_sTitle);
			
			lineComp.GetOnMouseEnter().Insert(OnLineMouseEnter);
			lineComp.GetOnMouseLeave().Insert(OnLineMouseLeave);
			
			if (status)
				SetServiceState(service.m_sId, status.m_Status);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set the ping value (formats -1 to " - ", 0 to " ..." and >999 value to "999+")
	//! \param pingInMs ping in milliseconds
	void SetPing(int pingInMs)
	{
		if (!m_wPingWidget)
			return;

		Color color = Color.FromInt(UIColors.WARNING.PackToInt());
		if (pingInMs < SCR_ServicesStatusHelper.PING_THRESHOLD_BAD)
			color = Color.FromInt(UIColors.HIGHLIGHTED.PackToInt());
		if (pingInMs < SCR_ServicesStatusHelper.PING_THRESHOLD_GOOD)
			color = Color.FromInt(UIColors.CONFIRM.PackToInt());
		
		string sPing;
		if (pingInMs == -1)
		{
			sPing = m_sNoPing;
			color = Color.FromInt(UIColors.WARNING.PackToInt());
		}
		
		else if (pingInMs == 0)
		{
			sPing = m_sUpdatingPing;
			color = Color.FromInt(UIColors.NEUTRAL_ACTIVE_STANDBY.PackToInt());
		}
		
		else if (pingInMs < 0 || pingInMs > SCR_ServicesStatusHelper.PING_MAX)
		{
			sPing = string.Format(m_sBigPing, SCR_ServicesStatusHelper.PING_MAX);
			color = Color.FromInt(UIColors.WARNING.PackToInt());
		}
		
		else
		{
			sPing = pingInMs.ToString();
		}

		m_wPingWidget.SetColor(color);
		m_wPingWidget.SetTextFormat(m_sPingText, sPing);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_BackendServiceDisplay> GetAllServices()
	{
		array<ref SCR_BackendServiceDisplay> services = {};
		if (m_ServicesPresets)
			services = m_ServicesPresets.GetServices();
		
		return services;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] status
	void SetAllServicesState(EServiceStatus status)
	{
		if (!m_ServicesPresets)
			return;
		
		foreach (SCR_BackendServiceDisplay serviceInfo : m_ServicesPresets.GetServices())
		{
			if (!SCR_ServicesStatusHelper.DisplayServiceOnCurrentPlatform(serviceInfo))
				continue;
			
			SetServiceState(serviceInfo.m_sId, status);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] serviceId
	//! \param[in] status
	void SetServiceState(string serviceId, EServiceStatus status)
	{
		Widget line = m_wLinesParentWidget.FindAnyWidget(serviceId);
		if (!line)
			return;

		SCR_ServicesStatusDialogLineComponent lineComp = SCR_ServicesStatusDialogLineComponent.FindComponent(line);
		if (!lineComp)
			return;
		
		if (lineComp.m_wBackground)
			SetStatusBackground(lineComp.m_wBackground, status);

		if (lineComp.m_wIconWidget)
			SetStatusImageAndColor(lineComp.m_wIconWidget, status);

		if (lineComp.m_wTitle)
			SetStatusText(lineComp.m_wTitle, status);
		
		lineComp.CacheStatus(status);
		
		if (m_Tooltip)
			m_Tooltip.ForceHidden();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commStatus
	//! \param[in] servicesStatus
	// The message reflects first of all the communication status with backend.
	// Should communication succeed, the message will then change depending on the state of services
	void UpdateServicesMessage(SCR_ECommStatus commStatus, EServiceStatus servicesStatus)
	{
		if (!m_wServicesMessage)
			return;

		switch(commStatus)
		{
			case SCR_ECommStatus.NOT_EXECUTED: 
			{
				m_wServicesMessage.SetText(m_sStatusesMessageInitializationError);
				break;
			}
			
			case SCR_ECommStatus.RUNNING:
			{
				m_wServicesMessage.SetText(m_sStatusesMessageAttemptingToConnect);
				break;
			}
			
			// Connection succeded, change the message based on the services available
			case SCR_ECommStatus.FINISHED:
			{
				switch(servicesStatus)
				{
					case EServiceStatus.RUNNING:
						m_wServicesMessage.SetText(m_sStatusesMessageRunning);
						break;
					
					case EServiceStatus.WARNING:
						m_wServicesMessage.SetText(m_sStatusesMessageWarning);
						break;
					
					case EServiceStatus.ERROR:
						m_wServicesMessage.SetText(m_sStatusesMessageError);
						break;
				}
				break;
			}
			
			case SCR_ECommStatus.FAILED: 
			{
				m_wServicesMessage.SetText(m_sStatusesMessageNoConnection);
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] pingAge
	// pingAge is in milliseconds
	void SetLastUpdateMessage(int pingAge)
	{
		if (!m_wLastUpdate)
			return;
		
		//No connection with backend
		if (pingAge < 0)
		{
			m_wLastUpdate.SetText(m_sLastUpdateMessageNoConnection);
			return;
		}
		
		int minutes = pingAge / 60000;
		int seconds = (pingAge / 1000) - (60 * minutes);
		
		if (minutes < 1)
			m_wLastUpdate.SetTextFormat(m_sLastUpdateMessageSeconds, seconds);
		else
			m_wLastUpdate.SetTextFormat(m_sLastUpdateMessageMinutes, minutes, seconds);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] countdown
	//! \param[in] visible
	void SetRefreshMessage(int countdown, bool visible = true)
	{
		if (!m_wRefreshMessage)
			return;
		
		m_wRefreshMessage.SetVisible(visible);
		if (!visible)
			return;

		m_wRefreshMessage.SetTextFormat(m_sRefreshText, countdown);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetStatusBackground(ImageWidget backgroundWidget, EServiceStatus serviceStatus)
	{
		SCR_ServicesStatusDialogComponent_Status status = GetStatus(serviceStatus, m_aStatuses);
		if (!status)
			return;

		backgroundWidget.SetColor(status.m_sBackgroundColor);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetStatusText(Widget textWidget, EServiceStatus serviceStatus)
	{
		SCR_ServicesStatusDialogComponent_Status status = GetStatus(serviceStatus, m_aStatuses);
		if (!status)
			return;
		
		textWidget.SetColor(status.m_sTextColor);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] iconWidget
	//! \param[in] serviceStatus
	//! \param[in] mainIcon
	void SetStatusImageAndColor(ImageWidget iconWidget, EServiceStatus serviceStatus, bool mainIcon = false)
	{
		SCR_ServicesStatusDialogComponent_Status status;
		
		if (mainIcon)
			status = GetStatus(serviceStatus, m_aMainStatuses);
		else
			status = GetStatus(serviceStatus, m_aStatuses);
		
		if (!status)
			return;

		iconWidget.LoadImageFromSet(0, status.m_sImageSet, status.m_sIcon);
		iconWidget.SetColor(status.m_sIconColor);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ServicesStatusDialogComponent_Status GetStatus(EServiceStatus serviceStatus, array<ref SCR_ServicesStatusDialogComponent_Status> statuses)
	{
		if (!statuses)
			return null;

		foreach (SCR_ServicesStatusDialogComponent_Status status : statuses)
		{
			if (status.m_Status == serviceStatus)
				return status;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineMouseEnter()
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineMouseLeave()
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		if (!tooltip || tooltip.GetTag() != m_sTooltipTag || !tooltip.GetHoverWidget())
		{
			m_Tooltip = null;
			return;
		}

		m_Tooltip = tooltip;
		
		SCR_ServicesStatusDialogLineComponent lineComp = SCR_ServicesStatusDialogLineComponent.FindComponent(tooltip.GetHoverWidget());
		if (!lineComp)
			return;
		
		EServiceStatus status = lineComp.GetStatus();
		string message;
		
		switch(status)
		{
			case EServiceStatus.ERROR:
			{
				message = m_sLegendError;
				break;
			}
			
			case EServiceStatus.WARNING:
			{
				message = m_sLegendWarning;
				break;
			}
			
			case EServiceStatus.RUNNING:
			{
				message = m_sLegendRunning;
				break;
			}
		}
		
		SCR_ScriptedWidgetTooltipContentBase content = m_Tooltip.GetContent();
		if (!content)
			return;
		
		content.SetMessage(message);
		
		SCR_ServicesStatusDialogComponent_Status statusPreset = GetStatus(status, m_aStatuses);
		if (statusPreset)
			content.SetMessageColor(statusPreset.m_sIconColor);
	}
}

[BaseContainerProps()]
class SCR_ServicesStatusDialogComponent_Status
{
	[Attribute(defvalue: UIConstants.ICONS_IMAGE_SET, params: "imageset")]
	ResourceName m_sImageSet;

	[Attribute(defvalue: SCR_Enum.GetDefault(EServiceStatus.ERROR), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EServiceStatus))]
	EServiceStatus m_Status;

	[Attribute(defvalue: UIConstants.ICON_OK)]
	string m_sIcon;

	[Attribute(defvalue: "1 1 1 1")]
	ref Color m_sIconColor;

	[Attribute(defvalue: "0 0 0 0")]
	ref Color m_sBackgroundColor;
	
	[Attribute(defvalue: "1 1 1 1")]
	ref Color m_sTextColor;
}

class SCR_ServicesStatusDialogLineComponent : SCR_EventHandlerComponent
{
	protected EServiceStatus m_eStatus;
	
	TextWidget m_wTitle;
	ImageWidget m_wBackground;
	ImageWidget m_wIconWidget;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wTitle = TextWidget.Cast(w.FindAnyWidget("Text"));
		m_wBackground = ImageWidget.Cast(w.FindAnyWidget("Background"));
		m_wIconWidget = ImageWidget.Cast(w.FindAnyWidget("Icon"));
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] status
	void CacheStatus(EServiceStatus status)
	{
		m_eStatus = status;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EServiceStatus GetStatus()
	{
		return m_eStatus;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] w
	//! \return
	static SCR_ServicesStatusDialogLineComponent FindComponent(notnull Widget w)
	{
		return SCR_ServicesStatusDialogLineComponent.Cast(w.FindHandler(SCR_ServicesStatusDialogLineComponent));
	}
}
