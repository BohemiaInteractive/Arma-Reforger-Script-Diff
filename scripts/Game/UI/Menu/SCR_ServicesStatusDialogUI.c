enum EServiceStatus
{
	RUNNING = 0,
	WARNING,
	ERROR,
};

class SCR_ServicesStatusDialogUI : SCR_ConfigurableDialogUi
{
	protected SCR_ServicesStatusDialogComponent m_ServicesStatusDialogComponent;

	protected static const int LAST_UPDATED_REFRESH_RATE = 500;		// ping age check frequency (is the ping old enough to enable the "Refresh" button?)
	
	protected SCR_InputButtonComponent m_Refresh;
	protected static bool m_bIsOpen;
	
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		m_bIsOpen = true;
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusUpdatedOpenDialog);
		
		m_Refresh = FindButton("Refresh");
		if (m_Refresh)
			m_Refresh.m_OnActivated.Insert(OnRefresh);

		m_ServicesStatusDialogComponent = SCR_ServicesStatusDialogComponent.Cast(GetContentLayoutRoot().FindHandler(SCR_ServicesStatusDialogComponent));

#ifdef WORKBENCH
		if (!m_ServicesStatusDialogComponent)
			Print("No SCR_ServicesStatusDialogComponent component found | " + __FILE__ + ": " + __LINE__, LogLevel.WARNING);
#endif
		
		OnRefresh();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuClose()
	{
		super.OnMenuClose();
		
		m_bIsOpen = false;
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusUpdated);
		GetGame().GetCallqueue().Remove(UpdateRefreshWidgets);
		GetGame().GetCallqueue().Remove(OnRefresh);
	}
	
	//------------------------------------------------------------------------------------------------
	//! refresh service statuses
	//! refresh UI through ServicesStatusDialogComponent
	protected void OnRefresh()
	{		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusUpdated);
		SCR_ServicesStatusHelper.RefreshPing();
		if (!SCR_ServicesStatusHelper.IsInRefreshCooldown())
			SCR_ServicesStatusHelper.ClearRefreshQueue();
		
		UpdateUI(SCR_ServicesStatusHelper.GetLastReceivedCommStatus());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusUpdated(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusUpdated);
		SCR_ServicesStatusHelper.ClearRefreshQueue();
		
		UpdateUI(status);
		
		// Automatic refresh
		GetGame().GetCallqueue().Remove(OnRefresh);
		GetGame().GetCallqueue().CallLater(OnRefresh, SCR_ServicesStatusHelper.AUTOMATIC_REFRESH_RATE);
	}
	
	//------------------------------------------------------------------------------------------------
	//! refresh UI through ServicesStatusDialogComponent
	protected void UpdateUI(SCR_ECommStatus status)
	{
		if (!m_ServicesStatusDialogComponent)
			return;
		
		EServiceStatus servicesStatus;
		
		switch(status)
		{
			case SCR_ECommStatus.NOT_EXECUTED: 
			{
				servicesStatus = EServiceStatus.ERROR;
				m_ServicesStatusDialogComponent.SetAllServicesState(EServiceStatus.ERROR);
				break;
			}
			
			case SCR_ECommStatus.RUNNING:
			{
				servicesStatus = EServiceStatus.WARNING;
				m_ServicesStatusDialogComponent.SetAllServicesState(EServiceStatus.WARNING);
			 	break;
			}
			
			case SCR_ECommStatus.FINISHED: 
			{
				servicesStatus = UpdateServiceStates();
				break;
			}
			
			case SCR_ECommStatus.FAILED:
			{
				servicesStatus = EServiceStatus.WARNING;
				m_ServicesStatusDialogComponent.SetAllServicesState(EServiceStatus.WARNING);
			 	break;
			}
		}
		
		// Update main Icon and message
		m_ServicesStatusDialogComponent.UpdateServicesMessage(status, servicesStatus);
	
		if (m_wImgTitleIcon)
			m_ServicesStatusDialogComponent.SetStatusImageAndColor(m_wImgTitleIcon, servicesStatus, true);

		// Update refresh widgets
		UpdatePingValueUI();
		UpdateRefreshWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates status of services and returns general status based on the number of active services
	protected EServiceStatus UpdateServiceStates()
	{
		EServiceStatus generalStatus = EServiceStatus.RUNNING; // General Status reporting! o7
		
		// Main status check
		ServiceStatusItem mainStatus = SCR_ServicesStatusHelper.GetMainStatus();
		if (!mainStatus || mainStatus.Status().IsEmpty())
		{
			generalStatus = EServiceStatus.ERROR;
			m_ServicesStatusDialogComponent.SetAllServicesState(generalStatus);
			return generalStatus;
		}
		
		// Update states of services
		int errors, warnings, totalServices;
		
		EServiceStatus status;
		foreach (SCR_BackendServiceDisplay serviceInfo : m_ServicesStatusDialogComponent.GetAllServices())
		{
			if (!SCR_ServicesStatusHelper.DisplayServiceOnCurrentPlatform(serviceInfo))
				continue;
			
			status = GetEnumStatus(serviceInfo.m_sServiceId);
			m_ServicesStatusDialogComponent.SetServiceState(serviceInfo.m_sId, status);
			
			if (status == EServiceStatus.ERROR)
				errors++;
			if (status == EServiceStatus.WARNING)
				warnings++;
			
			totalServices++;
		}

		if (warnings > 0)
			generalStatus = EServiceStatus.WARNING;
		if (errors + warnings == totalServices)
			generalStatus = EServiceStatus.ERROR;
		
		return generalStatus;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePingValueUI()
	{
		if (!m_ServicesStatusDialogComponent)
			return;
		
		m_ServicesStatusDialogComponent.SetPing(SCR_ServicesStatusHelper.GetPingValue());
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateRefreshWidgets()
	{
		GetGame().GetCallqueue().Remove(UpdateRefreshWidgets);
		GetGame().GetCallqueue().CallLater(UpdateRefreshWidgets, LAST_UPDATED_REFRESH_RATE);
		
		if (m_Refresh)
			m_Refresh.SetEnabled(SCR_ServicesStatusHelper.CanRefresh());

		if (!m_ServicesStatusDialogComponent)
			return;
		
		m_ServicesStatusDialogComponent.SetLastUpdateMessage(SCR_ServicesStatusHelper.GetPingAge());
		
		// Refresh countdown
		int seconds = ((SCR_ServicesStatusHelper.REFRESH_COOLDOWN - SCR_ServicesStatusHelper.GetPingAge()) * 0.001) + 1;
		m_ServicesStatusDialogComponent.SetRefreshMessage(seconds, !SCR_ServicesStatusHelper.CanRefresh() && seconds > 0);
	}

	//------------------------------------------------------------------------------------------------
	protected EServiceStatus GetEnumStatus(string wantedItemName)
	{
		ServiceStatusItem serviceItem = SCR_ServicesStatusHelper.GetStatusByName(wantedItemName);
		if (!serviceItem)
			return EServiceStatus.WARNING;

		return GetStatusFromString(serviceItem.Status());
	}

	//------------------------------------------------------------------------------------------------
	protected EServiceStatus GetStatusFromString(string status)
	{
		status.ToLower();
		
		if (status == SCR_ServicesStatusHelper.STATUS_OK)
			return EServiceStatus.RUNNING;

		if (status == SCR_ServicesStatusHelper.STATUS_ERROR)
			return EServiceStatus.ERROR;

		return EServiceStatus.WARNING;
	}

	//------------------------------------------------------------------------------------------------
	// TODO: look at the code where this is called, it's working weirdly
	// the waiting-for-display logic should happen in MainMenuUI, the refresh logic in here
	static void OpenIfServicesAreNotOK()
	{
		// returns false even if enabled but without internet connection for now
		if (!SCR_ServicesStatusHelper.IsBackendEnabled())
		{
			// SCR_CommonDialogs.CreateServicesStatusDialog();
			return;
		}

		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusUpdatedOpenDialog);
		SCR_ServicesStatusHelper.RefreshPing();
	}	
		
	//------------------------------------------------------------------------------------------------
	protected static void OnCommStatusUpdatedOpenDialog(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusUpdatedOpenDialog);
		
		ServiceStatusItem mainStatus = SCR_ServicesStatusHelper.GetMainStatus();
		
		bool statusIssues = status == SCR_ECommStatus.NOT_EXECUTED || !mainStatus || mainStatus.Status() != SCR_ServicesStatusHelper.STATUS_OK;
		if (statusIssues && !m_bIsOpen && GetGame().m_bIsMainMenuOpen) //Open in core menus only
			SCR_CommonDialogs.CreateServicesStatusDialog();
	}
}
