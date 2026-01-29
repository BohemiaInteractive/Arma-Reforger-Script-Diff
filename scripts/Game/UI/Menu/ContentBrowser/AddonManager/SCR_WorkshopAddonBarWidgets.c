class SCR_WorkshopAddonBarWidgets
{
	static const ResourceName s_sLayout = "{50B16D175FA6FD08}UI/layouts/Menus/ContentBrowser/AddonManager/AddonBar/AddonBar_Refactored.layout";
	ResourceName GetLayout() { return s_sLayout; }

	Widget m_wUpdateButton;
	SCR_CoreMenuHeaderButtonComponent m_UpdateButtonComponent;

	Widget m_wPresetsButton;
	SCR_CoreMenuHeaderButtonComponent m_PresetsButtonComponent;

	bool Init(Widget root)
	{
		m_wUpdateButton = root.FindAnyWidget("UpdateButton");
		m_UpdateButtonComponent = SCR_CoreMenuHeaderButtonComponent.Cast(m_wUpdateButton.FindHandler(SCR_CoreMenuHeaderButtonComponent));

		m_wPresetsButton = root.FindAnyWidget("PresetsButton");
		m_PresetsButtonComponent = SCR_CoreMenuHeaderButtonComponent.Cast(m_wPresetsButton.FindHandler(SCR_CoreMenuHeaderButtonComponent));

		return true;
	}
}
