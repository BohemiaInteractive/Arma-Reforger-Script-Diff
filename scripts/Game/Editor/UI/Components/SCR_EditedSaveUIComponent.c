/*!
UI Component for handling edited save widget 
Can: Display currently edited save, show save state (changes), trigger upload
*/
class SCR_EditedSaveUIComponent : SCR_ScriptedWidgetComponent
{
	protected const string WIDGET_NAME = "m_Name";
	protected const string WIDGET_UPLOAD_BTN = "m_UploadButton";
	protected const string WIDGET_SAVE_BACKGROUND = "m_IconBackground";
	
	protected const string TOOLTIP_PUBLISH = "#AR-Editor_SaveLoad_TooltipPublish";
	protected const string TOOLTIP_PUBLISH_EMPTY = "#AR-Editor_SaveLoad_TooltipMissingSave";
	protected const string TOOLTIP_PUBLISH_DISABLED_DOWNLOADED = "#AR-Editor_SaveLoad_TooltipDownloadedPublish";
	
	protected const string DEFAULT_NAME = "#AR-Editor_SaveLoad_NewSaveFallback";
	
	protected TextWidget m_wName;
	protected ImageWidget m_wSaveBackground;
	
	protected ButtonWidget m_wUploadButton;
	protected SCR_ModularButtonComponent m_UploadButton;
	protected SCR_DynamicIconComponent m_UploadButtonIcon;
	
	protected bool m_bCanUpload;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		super.HandlerAttached(w);
		
		// Setup refs
		m_wName = TextWidget.Cast(w.FindAnyWidget(WIDGET_NAME));
		m_wSaveBackground = ImageWidget.Cast(w.FindAnyWidget(WIDGET_SAVE_BACKGROUND));
		
		m_wUploadButton = ButtonWidget.Cast(w.FindAnyWidget(WIDGET_UPLOAD_BTN));
		m_UploadButton = SCR_ModularButtonComponent.Cast(m_wUploadButton.FindHandler(SCR_ModularButtonComponent));
		m_UploadButtonIcon = SCR_DynamicIconComponent.Cast(m_wUploadButton.FindHandler(SCR_DynamicIconComponent));
		
		// Setup state
		if (!GetGame().InPlayMode())
			return;
		
		ClearEditedSave();
		
		/*
		SCR_SaveWorkshopManager saveWorkshopManager = SCR_SaveWorkshopManager.GetInstance();
		WorldSaveItem saveItem;
		string saveName = saveWorkshopManager.GetCurrentSave(saveItem);
		
		if (saveItem)
			OnEditedSaveChange(saveName);
		
		// Setup callbacks 
		saveWorkshopManager.GetOnCurrentSaveChanged().Insert(OnEditedSaveChange);
		*/
		
		m_UploadButton.m_OnClicked.Insert(OnUploadButtonClick);
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		SCR_ListEntryHelper.UpdateMouseButtonColor(m_UploadButton, !m_bCanUpload, true);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		SCR_ListEntryHelper.UpdateMouseButtonColor(m_UploadButton, !m_bCanUpload, m_UploadButton.GetFocused());
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		//++ Ensure the tooltip is only hijacked if its a publish tooltip
		string tag = tooltip.GetTag();
		if (tag != "publish_action" && tag != "publish_disabled_empty" && tag != "publish_disabled_downloaded")
			return;
		
		//WorldSaveItem saveItem;
		//SCR_SaveWorkshopManager.GetInstance().GetCurrentSave(saveItem);
		string message = TOOLTIP_PUBLISH_EMPTY;
		
		/*
		if (saveItem)
		{
			if (SCR_SaveWorkshopManager.CanOverrideSave(saveItem))
				message = TOOLTIP_PUBLISH;
			else
				message = TOOLTIP_PUBLISH_DISABLED_DOWNLOADED;
		}
		*/
		
		tooltip.GetContent().SetMessage(message);
			
		if (m_bCanUpload)
			tooltip.GetContent().SetMessageColor(UIColors.NEUTRAL_INFORMATION);
		else 
			tooltip.GetContent().SetMessageColor(UIColors.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Reaction to changing of current editing save - name should be changed accordingly
	protected void OnEditedSaveChange(string fileName)
	{
		//string name = GetGame().GetSaveManager().GetCustomName(fileName);
		
		m_wName.SetText(fileName);
		
		UpdatePublishButtonState();
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup current tooltip based on current save state
	protected void UpdatePublishButtonState()
	{
		/*WorldSaveItem saveItem;
		SCR_SaveWorkshopManager.GetInstance().GetCurrentSave(saveItem);
		
		m_bCanUpload = saveItem && SCR_SaveWorkshopManager.CanOverrideSave(saveItem);
		
		SCR_ListEntryHelper.UpdateMouseButtonColor(m_UploadButton, !m_bCanUpload, m_UploadButton.GetFocused());
		
		if (saveItem)
		{
			m_wName.SetColor(UIColors.NEUTRAL_INFORMATION);
			m_wSaveBackground.SetColor(UIColors.CONTRAST_COLOR);
		}
		else
		{
			m_wName.SetColor(UIColors.WHITE_DISABLED);
			m_wSaveBackground.SetColor(UIColors.BACKGROUND_HOVERED);
		}
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnUploadButtonClick(SCR_ModularButtonComponent button)
	{
		if (!m_bCanUpload)
			return;
		
		// Check linked account
		if (!BohemiaAccountApi.IsLinked())
		{
			SCR_LoginProcessDialogUI loginDialog = SCR_LoginProcessDialogUI.CreateLoginDialog();
			loginDialog.GetCallback().SetOnSuccess(OnLoginSuccess);
			return;
		}
		
		new SCR_PublishSaveDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLoginSuccess()
	{
		new SCR_PublishSaveDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearEditedSave()
	{
		m_wName.SetText(DEFAULT_NAME);
		UpdatePublishButtonState();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ModularButtonComponent GetUploadButton()
	{
		return m_UploadButton;
	}
}