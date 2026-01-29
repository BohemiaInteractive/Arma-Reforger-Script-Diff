class ReportDialogUI: DialogUI
{
	protected SCR_WorkshopItem m_WorkshopItem;
	
	protected ContentBrowserDetailsMenu m_DetailsMenu;
		
	protected SCR_ComboBoxComponent m_ReasonCombo;
	protected SCR_EditBoxComponent m_InputField;
	
	protected Widget m_wDialogWindow;
	
	protected SCR_LoadingOverlayDialog m_Overlay;
	
	protected bool m_bAcceptInput = true;
	
	protected ref SCR_WorkshopItemAction m_ReportAction;
	
	protected bool m_bShouldEnableConfirmButton;
	
	protected ref ScriptInvokerBool m_OnReportSuccess;
		
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		Widget root = GetRootWidget();
		
		m_InputField = SCR_EditBoxComponent.GetEditBoxComponent("InputField", root);
		if (m_InputField)
			m_InputField.m_OnChanged.Insert(OnFieldChanged);
		
		m_ReasonCombo = SCR_ComboBoxComponent.GetComboBoxComponent("ReasonCombo", root);
		if (m_ReasonCombo)
		{
			m_ReasonCombo.ClearAll();
			string reportTypeStr = SCR_WorkshopUiCommon.GetReportTypeString(0);
			int i = 1;
			while (!reportTypeStr.IsEmpty())
			{
				m_ReasonCombo.AddItem(reportTypeStr);
				reportTypeStr = SCR_WorkshopUiCommon.GetReportTypeString(i);
				i++;
			}
			m_ReasonCombo.SetCurrentItem(0);
		}
		
		m_wDialogWindow = root.FindAnyWidget("DialogBase0");
		
		// Disable the confirm button initially because text is empty
		m_Confirm.SetEnabled(false);
		
		if(m_InputField)
		{
			m_InputField.m_OnWriteModeLeave.Insert(OnWriteModeLeaveInternal);
			m_InputField.m_OnTextChange.Insert(OnTextChange);
		}
		
		SCR_InputButtonComponent tos = SCR_InputButtonComponent.GetInputButtonComponent("ToS", GetRootWidget());
		if (tos)
			tos.m_OnActivated.Insert(OnTos);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		if (m_InputField)
			GetGame().GetWorkspace().SetFocusedWidget(m_InputField.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		GetGame().GetInputManager().ActivateContext("InteractableDialogContext");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		if (!m_bAcceptInput || !m_WorkshopItem)
			return;
		
		m_OnConfirm.Invoke();
		//super.OnConfirm(); // we don't want to close the dialog
		
		// Fetch feedback data

		int category;
		int reason;
		string content = "";
		
		if (m_ReasonCombo)
		{
			category = m_ReasonCombo.GetCurrentIndex();
			reason = category;
		}
		
		if (m_InputField)
			content = m_InputField.GetValue();
		
		// Show loading screen
		m_Overlay = SCR_LoadingOverlayDialog.Create();
		
		// Reactivating previous report 
		if (m_ReportAction)
		{
			m_ReportAction.Reactivate();
			return;
		}
		
		// Report
		m_ReportAction = m_WorkshopItem.Report(reason, content);
		
		m_ReportAction.m_OnCompleted.Insert(OnReportSuccess);
		m_ReportAction.m_OnFailed.Insert(OnReportFailed);
		
		m_ReportAction.Activate();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnReportSuccess()
	{
		if (!m_WorkshopItem)
			return;
		
		if (m_OnReportSuccess)
			m_OnReportSuccess.Invoke(false);
		
		m_WorkshopItem.DeleteLocally();
		m_WorkshopItem.SetSubscribed(false);
		
		// Cancel download 
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
		{
			SCR_WorkshopItemActionDownload action = mgr.GetActionOfItem(m_WorkshopItem);
			if (action)
				action.Cancel();
		}
		
		if (m_Overlay)
			m_Overlay.Close();
		
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnReportFailed(SCR_WorkshopItemAction action)
	{
		if (m_Overlay)
			m_Overlay.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetAcceptInput(bool accept)
	{
		m_bAcceptInput = accept;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if content (textbox) is empty
	protected bool IsContentEmpty()
	{
		if (m_InputField)
		{
			string txt = m_InputField.GetValue();
			return txt.Length() <= 0;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnWriteModeLeaveInternal(string text)
	{
		OnTextChange(text);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTextChange(string text)
	{
		if (!m_Confirm || !m_InputField)
			return;
		
		m_Confirm.SetEnabled(!m_InputField.GetValue().IsEmpty());
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnFieldChanged(SCR_EditBoxComponent comp, string text)
	{
		if (m_Confirm)
			m_Confirm.SetEnabled(text.Length() > 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTos()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PrivacyPolicyMenu);
		//GetGame().GetPlatformService().OpenBrowser(GetGame().GetBackendApi().GetLinkItem("Link_PrivacyPolicy"));
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWorkshopItem(SCR_WorkshopItem item)
	{
		m_WorkshopItem = item;	
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnReportSuccess()
	{
		if (!m_OnReportSuccess)
			m_OnReportSuccess = new ScriptInvokerBool();
		
		return m_OnReportSuccess;
	}
}