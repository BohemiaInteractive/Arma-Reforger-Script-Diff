/*
Specific workshop item actions, inherited from base action class.
*/

//-----------------------------------------------------------------------------------------------
//! Action for downloading latest dependencies. Doesn't download this addon, but only downloads dependencies.
class SCR_WorkshopItemActionDownloadDependenciesLatest : SCR_WorkshopItemActionComposite
{	
	// Optionally it is possible to provide dependencies externally,
	// In this case only these will be downloaded.
	ref array<ref SCR_WorkshopItem> m_aProvidedDependencies;
	
	//-----------------------------------------------------------------------------------------------
	//! You must provide an array of dependencies to download.
	void SCR_WorkshopItemActionDownloadDependenciesLatest(SCR_WorkshopItem wrapper, array<ref SCR_WorkshopItem> dependencies)
	{
		if (dependencies.Count() > 0)
		{
			// Copy the provided dependencies
			m_aProvidedDependencies = new array<ref SCR_WorkshopItem>;
			foreach (auto dep : dependencies)
				m_aProvidedDependencies.Insert(dep);
			
			// Create actions for all dependencies
			#ifdef WORKSHOP_DEBUG
			_print("New() Creating actions:");
			#endif
			m_aActions.Clear();
			foreach (SCR_WorkshopItem dep : m_aProvidedDependencies)
			{
				if (!dep.GetOffline() || dep.GetUpdateAvailable())
				{
					#ifdef WORKSHOP_DEBUG
					_print(string.Format("  Dependency: %1 - Creating action", dep.GetName()));
					#endif
					
					SCR_WorkshopItemAction action = dep.DownloadLatestVersion();
					m_aActions.Insert(action);
				}
				else
				{
					#ifdef WORKSHOP_DEBUG
					_print(string.Format("  Dependency: %1 - Download is not required.", dep.GetName()));
					#endif
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnActivate()
	{	
		bool success = true;
		
		// Activate all created actions
		foreach (auto action : m_aActions)
		{
			if (action.IsPaused())
				success = success && action.Resume();
			else if (action.IsInactive())
				success = success && action.Activate();
		}
			
		return success;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnReactivate()
	{
		return OnActivate();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnCancel()
	{
		CancelAll();
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnPause()
	{
		PauseAll();
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnResume()
	{
		ResumeAll();
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	override void Internal_Update(float timeSlice)
	{
		if (!m_Wrapper)
			return;
		
		// Check if comms have failed
		if (m_State != STATE_COMPLETED && m_State != STATE_FAILED)
		{
			if (m_Wrapper.GetRequestFailed())
				this.Fail();
		}
		
		// Unregister dependnecy actions which were canceled
		array<SCR_WorkshopItemAction> removeActions = new array<SCR_WorkshopItemAction>;
		foreach (SCR_WorkshopItemAction action : m_aActions)
		{
			if (action.IsCanceled() || action.IsPaused() || action.IsFailed()) // todo should paused actions be removed too?
				removeActions.Insert(action);
		}
		UnregisterActions(removeActions);
		
		// Complete when all actions are completed
		if (AllCompleted())
		{
			this.Complete();
		}
	}
};

//-----------------------------------------------------------------------------------------------
//! Action for reporting an item
class SCR_WorkshopItemActionReportBase : SCR_WorkshopItemAction
{
	ref BackendCallback m_Callback;	
	
	//-----------------------------------------------------------------------------------------------
	protected bool OnActivateInternal();
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnActivate()
	{
		m_Callback = new BackendCallback();
		m_Callback.SetOnSuccess(Callback_OnSuccess);
		m_Callback.SetOnError(Callback_OnError);
		
		bool success = OnActivateInternal();	
		return success;
	}
	

	//-----------------------------------------------------------------------------------------------
	protected override bool OnReactivate()
	{
		return this.OnActivate();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnError(BackendCallback cb)
	{
		Fail();
		if (cb.GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT)
		{
			SCR_CommonDialogs.CreateTimeoutOkDialog();
		}
		else
		{
			SCR_CommonDialogs.CreateRequestErrorDialog();
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnSuccess()
	{
		this.Complete();
	}
};

//-----------------------------------------------------------------------------------------------
//! Action for reporting an item
class SCR_WorkshopItemActionReport : SCR_WorkshopItemActionReportBase
{
	protected EWorkshopReportType m_eReportType;
	protected string m_sMessage;	
	
	//-----------------------------------------------------------------------------------------------
	void SCR_WorkshopItemActionReport(SCR_WorkshopItem wrapper, EWorkshopReportType eReport, string sMessage)
	{
		m_eReportType = eReport;
		m_sMessage = sMessage;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnActivateInternal()
	{
		return m_Wrapper.Internal_Report(m_eReportType, m_sMessage, m_Callback);
	}
};



//-----------------------------------------------------------------------------------------------
//! Action for canceling report of an item
class SCR_WorkshopItemActionCancelReport : SCR_WorkshopItemActionReportBase
{	
	protected override bool OnActivateInternal()
	{
		return m_Wrapper.Internal_CancelReport(m_Callback);
	}
};


//-----------------------------------------------------------------------------------------------
//! Action for blocking author
class SCR_WorkshopItemActionAddAuthorBlock : SCR_WorkshopItemActionReportBase
{	
	protected override bool OnActivateInternal()
	{
		return m_Wrapper.Internal_AddAuthorBlock(m_Callback);
	}
};

//-----------------------------------------------------------------------------------------------
//! Action for removing author mod content blocking
class SCR_WorkshopItemActionRemoveAuthorBlock : SCR_WorkshopItemActionReportBase
{	
	protected override bool OnActivateInternal()
	{
		return m_Wrapper.Internal_RemoveAuthorBlock(m_Callback);
	}
};