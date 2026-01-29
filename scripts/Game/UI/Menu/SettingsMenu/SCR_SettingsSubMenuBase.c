class SCR_SettingsSubMenuBase: SCR_SubMenuBase
{
	protected ref array<ref SCR_SettingsBindingBase> m_aSettingsBindings = {};
	protected ScrollLayoutWidget m_wScroll;
	protected bool m_bLoadingSettings;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		m_wScroll = ScrollLayoutWidget.Cast(m_wRoot.FindAnyWidget("ScrollLayout0"));
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();
		GetGame().SaveUserSettings();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		// Reset the focused Widget, to prevent that a Widget from a previous tab is still focused and controlled.
		GetGame().GetWorkspace().SetFocusedWidget(null);
		
		if (m_wScroll)
			m_wScroll.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadSettings(bool forceLoadSettings = false, bool addEntryChangedEventHandler = true)
	{
		if (!m_wScroll)
			return;

		m_bLoadingSettings = true;
		foreach (SCR_SettingsBindingBase bind : m_aSettingsBindings)
		{
			bind.LoadEntry(m_wScroll, forceLoadSettings, addEntryChangedEventHandler);
			if (addEntryChangedEventHandler)
				bind.GetEntryChangedInvoker().Insert(OnMenuItemChanged);
		}
		m_bLoadingSettings = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuItemChanged(SCR_SettingsBindingBase binding)
	{
		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void HideMenuItem(string widgetName)
	{
		Widget w = GetRootWidget().FindAnyWidget(widgetName);
		if (w)
			w.SetVisible(false);
	}
};