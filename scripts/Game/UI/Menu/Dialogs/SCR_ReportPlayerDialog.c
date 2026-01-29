class SCR_ReportPlayerDialog : SCR_ConfigurableDialogUi
{
	protected int m_iReportedPlayedID;
	
	protected ref array<SCR_ModularButtonComponent> m_aReasonButtons = {};
	
	protected SCR_EReportReason m_eReason;
	
	//------------------------------------------------------------------------------------------------
	void SCR_ReportPlayerDialog(int reportedPlayedID)
	{
		m_iReportedPlayedID = reportedPlayedID;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "report_player", this);
	}

    //------------------------------------------------------------------------------------------------
    override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
    {
		super.OnMenuOpen(preset);
		
		SCR_InputButtonComponent confirm = FindButton("confirm_report");
		if (confirm)
			confirm.m_OnActivated.Insert(OnConfirmReport);
		
		Widget reasonsLayout = m_wRoot.FindAnyWidget("ReasonsLayout");
		if (!reasonsLayout)
			return;
		
		Widget child = reasonsLayout.GetChildren();
		if (!child)
			return;
		
		SCR_ModularButtonComponent modularComponent;
		while (child)
		{
			modularComponent = SCR_ModularButtonComponent.Cast(child.FindHandler(SCR_ModularButtonComponent));
			if (!modularComponent)
				continue;
			
			modularComponent.m_OnToggled.Insert(OnReasonToggled);
			modularComponent.m_OnFocus.Insert(OnReasonFocused);
			m_aReasonButtons.Insert(modularComponent);
			
			child = child.GetSibling();
		}
		
		RichTextWidget reportedPlayerName = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ReportedPlayerName"));
		if (!reportedPlayerName)
			return;
		
		string name = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(m_iReportedPlayedID);
		reportedPlayerName.SetText(name);
    }
	
	//------------------------------------------------------------------------------------------------
	void OnConfirmReport()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return;
		
		SocialComponent social = SocialComponent.Cast(controller.FindComponent(SocialComponent));
		if (!social)
			return;
		
		social.m_OnReportPlayerFinishInvoker.Insert(OnReportResult);
		social.ReportPlayer(m_iReportedPlayedID, m_eReason);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnReasonToggled(SCR_ModularButtonComponent component, bool isToggled)
	{
		if (!isToggled)
			return;
		
		foreach (SCR_ModularButtonComponent comp : m_aReasonButtons)
		{
			if (comp == component)
				continue;
			
			comp.SetToggled(false);
		}
		
		Widget reasonWidget = component.GetRootWidget();
		if (!reasonWidget)
			return;
		
		SCR_ReportReasonComponent reasonComponent = SCR_ReportReasonComponent.Cast(reasonWidget.FindHandler(SCR_ReportReasonComponent));
		if (!reasonComponent)
			return;
		
		m_eReason = reasonComponent.GetReason();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnReasonFocused(SCR_ModularButtonComponent component)
	{
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.KEYBOARD)
			return;

		Widget reasonWidget = component.GetRootWidget();
		if (!reasonWidget)
			return;
		
		SCR_ReportReasonComponent reasonComponent = SCR_ReportReasonComponent.Cast(reasonWidget.FindHandler(SCR_ReportReasonComponent));
		if (!reasonComponent)
			return;
		
		m_eReason = reasonComponent.GetReason();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnReportResult(int playerID, bool success)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return;
		
		SocialComponent social = SocialComponent.Cast(controller.FindComponent(SocialComponent));
		if (!social)
			return;
		
		social.m_OnReportPlayerFinishInvoker.Remove(OnReportResult);
		
	    if (success)
	    {
	       SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "report_player_confirmation");
			Close();
	    }
	    else
	    {
	       SCR_BlockedUsersDialogUI dialog = new SCR_BlockedUsersDialogUI();
	       SCR_ConfigurableDialogUi.CreateFromPreset(SCR_AccountWidgetComponent.BLOCKED_USER_DIALOG_CONFIG, "block_failed_general", dialog);
			Close();
	    }
	}
};

class SCR_ReportReasonComponent : ScriptedWidgetComponent
{
	[Attribute(SCR_EReportReason.CHEATING.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EReportReason))]
	protected SCR_EReportReason m_eReason;
	
	[Attribute("Content", UIWidgets.EditBox, "Widget in which the text of the reason is placed")]
	protected string m_sReasonTextWidget;
	
	[Attribute("", UIWidgets.EditBox, "ID of the localized string for the report reason")]
	protected string m_sReasonLocalizedTextID;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{		
		TextWidget textWidget = TextWidget.Cast(w.FindAnyWidget(m_sReasonTextWidget));
		if (!textWidget)
			return;
		textWidget.SetText(m_sReasonLocalizedTextID);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EReportReason GetReason()
	{
		return m_eReason;
	}
}