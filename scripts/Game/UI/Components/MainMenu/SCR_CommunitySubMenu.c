class SCR_CommunitySubMenu : SCR_SubMenuBase
{
	[Attribute("{6FFF0601A48C7FEE}UI/layouts/Menus/MainMenu/CommunityNotificationButton.layout", UIWidgets.ResourceNamePicker, "", "layout")]
	protected ResourceName m_sNotificationsLayout;

	[Attribute("Send")]
	protected string m_sSendButtonName;
	[Attribute("Feedback")]
	protected string m_sFeedbackEditboxName;
	[Attribute("Address")]
	protected string m_sMailEditboxName;
	[Attribute("Notifications")]
	protected string m_sNotificationName;
	[Attribute("Type")]
	protected string m_sTypeComboBoxName;
	[Attribute("Category")]
	protected string m_sCategoryComboBoxName;

	protected SCR_ButtonBaseComponent m_Send;
	protected SCR_EditBoxComponent m_Feedback;
	protected SCR_ComboBoxComponent m_FeedbackType;
	protected SCR_ComboBoxComponent m_FeedbackCategory;
	
	SCR_InputButtonComponent m_TermsOfService;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		SetupNotifications();

		m_Send = SCR_ButtonBaseComponent.GetButtonBase(m_sSendButtonName, m_wRoot);
		if (m_Send)
		{
			m_Send.m_OnClicked.Insert(OnSendFeedback);
			m_Send.SetEnabled(false, false);
		}

		m_Feedback = SCR_EditBoxComponent.GetEditBoxComponent(m_sFeedbackEditboxName, m_wRoot);

		m_TermsOfService = m_DynamicFooter.FindButton("ToS");
		if (m_TermsOfService)
		{
			m_TermsOfService.SetVisible(true);
			m_TermsOfService.m_OnActivated.Insert(OnTos);
		}

		SetupCategories();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		if (m_Send)
			m_Send.SetEnabled(m_Feedback.GetValue() != string.Empty && SCR_FeedbackDialogUI.CanSendFeedback());
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();
		
		SCR_FeedbackDialogUI.ClearFeedback();

		if (m_TermsOfService)
			m_TermsOfService.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabRemove()
	{
		super.OnTabRemove();
		
		SCR_FeedbackDialogUI.ClearFeedback();
	}
	
	// Setup type and category combos according to SCR_FeedbackDialogUI constants
	//------------------------------------------------------------------------------------------------
	protected void SetupCategories()
	{
		m_FeedbackCategory = SCR_ComboBoxComponent.GetComboBoxComponent(m_sCategoryComboBoxName, m_wRoot);
		m_FeedbackType = SCR_ComboBoxComponent.GetComboBoxComponent(m_sTypeComboBoxName, m_wRoot);
		if (!m_FeedbackCategory || !m_FeedbackType)
			return;

		m_FeedbackCategory.ClearAll();
		m_FeedbackType.ClearAll();


		foreach (string s : SCR_FeedbackDialogUI.CATEGORY_NAMES)
		{
			m_FeedbackCategory.AddItem(s);
		}

		foreach (string s : SCR_FeedbackDialogUI.TYPE_NAMES)
		{
			m_FeedbackType.AddItem(s);
		}

		m_FeedbackType.SetCurrentItem(0);
		m_FeedbackCategory.SetCurrentItem(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupNotifications()
	{
		Widget notifications = m_wRoot.FindAnyWidget(m_sNotificationName);
		if (!notifications)
			return;

		array<ref SCR_NewsEntry> entries = {};
		MainMenuUI.GetNotificationEntries(entries);
		foreach (SCR_NewsEntry entry : entries)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sNotificationsLayout, notifications);
			if (!w)
				continue;

			SCR_NewsTileComponent tile = SCR_NewsTileComponent.Cast(w.FindHandler(SCR_NewsTileComponent));
			if (!tile)
				continue;

			tile.ShowTile(entry);
		}
	}
	
	// Copy code from feedback dialog
	//------------------------------------------------------------------------------------------------
	protected void OnSendFeedback()
	{
		if (!m_Feedback || !m_FeedbackType || !m_FeedbackCategory)
			return;

		string content = m_Feedback.GetValue();

		// Clear feedback window
		m_Feedback.SetValue(string.Empty);

		// Send feedback
		SCR_FeedbackDialogUI.SendFeedback(content, m_FeedbackType.GetCurrentIndex(), m_FeedbackCategory.GetCurrentIndex(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTos()
	{
		MenuBase base = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PrivacyPolicyMenu);
		if (!base)
			return;
		
		GetGame().GetWorkspace().SetFocusedWidget(base.GetRootWidget());
	}
}
