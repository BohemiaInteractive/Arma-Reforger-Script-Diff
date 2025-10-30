// Layout file: UI/layouts/Menus/ContentBrowser/AddonManager/AddonListTab/ListAddonsSubMenu.layout
// Layout file: UI/layouts/Menus/ContentBrowser/AddonManager/AddonListTab/ListAddonsSubMenu.layout
class SCR_ListAddonsSubMenuWidgets
{
	protected static const ResourceName LAYOUT = "{0FFE02F22D314775}UI/layouts/Menus/ContentBrowser/AddonManager/AddonListTab/ListAddonsSubMenu.layout";

	ButtonWidget m_wToolsButton;
	SCR_ModularButtonComponent m_ToolsButtonComponent0;
	SCR_DynamicIconComponent m_ToolsButtonComponent1;

	SizeLayoutWidget m_wDisabledAddonsPanel;

	ButtonWidget m_wValidateRepairAllDisabled;
	SCR_ModularButtonComponent m_ValidateRepairAllDisabledComponent0;
	SCR_DynamicIconComponent m_ValidateRepairAllDisabledComponent1;


	ButtonWidget m_wDeleteAllDisabled;
	SCR_ModularButtonComponent m_DeleteAllDisabledComponent0;
	SCR_DynamicIconComponent m_DeleteAllDisabledComponent1;


	ButtonWidget m_wButtonEnableAll;
	SCR_ModularButtonComponent m_ButtonEnableAllComponent0;
	SCR_DynamicIconComponent m_ButtonEnableAllComponent1;

	VerticalLayoutWidget m_wDisabledAddonsList;
	SizeLayoutWidget m_wEnabledAddonsPanel;

	ButtonWidget m_wButtonDisableAll;
	SCR_ModularButtonComponent m_ButtonDisableAllComponent0;
	SCR_DynamicIconComponent m_ButtonDisableAllComponent1;


	ButtonWidget m_wValidateRepairAllEnabled;
	SCR_ModularButtonComponent m_ValidateRepairAllEnabledComponent0;
	SCR_DynamicIconComponent m_ValidateRepairAllEnabledComponent1;


	ButtonWidget m_wDeleteAllEnabled;
	SCR_ModularButtonComponent m_DeleteAllEnabledComponent0;
	SCR_DynamicIconComponent m_DeleteAllEnabledComponent1;

	VerticalLayoutWidget m_wEnabledAddonsList;

	//------------------------------------------------------------------------------------------------
	bool Init(notnull Widget root)
	{
		m_wToolsButton = ButtonWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.TopBar.Content.HorizontalContent.m_ToolsButton"));
		m_ToolsButtonComponent0 = SCR_ModularButtonComponent.Cast(m_wToolsButton.FindHandler(SCR_ModularButtonComponent));
		m_ToolsButtonComponent1 = SCR_DynamicIconComponent.Cast(m_wToolsButton.FindHandler(SCR_DynamicIconComponent));

		m_wDisabledAddonsPanel = SizeLayoutWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_DisabledAddonsPanel"));

		m_wValidateRepairAllDisabled = ButtonWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_DisabledAddonsPanel.Overlay0.VerticalLayout0.TopBarHeight.TopBar.Content.HorizontalLayout0.m_ValidateRepairAllDisabled"));
		m_ValidateRepairAllDisabledComponent0 = SCR_ModularButtonComponent.Cast(m_wValidateRepairAllDisabled.FindHandler(SCR_ModularButtonComponent));
		m_ValidateRepairAllDisabledComponent1 = SCR_DynamicIconComponent.Cast(m_wValidateRepairAllDisabled.FindHandler(SCR_DynamicIconComponent));


		m_wDeleteAllDisabled = ButtonWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_DisabledAddonsPanel.Overlay0.VerticalLayout0.TopBarHeight.TopBar.Content.HorizontalLayout0.m_DeleteAllDisabled"));
		m_DeleteAllDisabledComponent0 = SCR_ModularButtonComponent.Cast(m_wDeleteAllDisabled.FindHandler(SCR_ModularButtonComponent));
		m_DeleteAllDisabledComponent1 = SCR_DynamicIconComponent.Cast(m_wDeleteAllDisabled.FindHandler(SCR_DynamicIconComponent));


		m_wButtonEnableAll = ButtonWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_DisabledAddonsPanel.Overlay0.VerticalLayout0.TopBarHeight.TopBar.Content.HorizontalLayout0.m_ButtonEnableAll"));
		m_ButtonEnableAllComponent0 = SCR_ModularButtonComponent.Cast(m_wButtonEnableAll.FindHandler(SCR_ModularButtonComponent));
		m_ButtonEnableAllComponent1 = SCR_DynamicIconComponent.Cast(m_wButtonEnableAll.FindHandler(SCR_DynamicIconComponent));

		m_wDisabledAddonsList = VerticalLayoutWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_DisabledAddonsPanel.Overlay0.VerticalLayout0.Content.Scroll.m_DisabledAddonsList"));
		m_wEnabledAddonsPanel = SizeLayoutWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_EnabledAddonsPanel"));

		m_wButtonDisableAll = ButtonWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_EnabledAddonsPanel.Overlay0.VerticalLayout0.TopBarHeight.TopBar.Content.HorizontalLayout0.m_ButtonDisableAll"));
		m_ButtonDisableAllComponent0 = SCR_ModularButtonComponent.Cast(m_wButtonDisableAll.FindHandler(SCR_ModularButtonComponent));
		m_ButtonDisableAllComponent1 = SCR_DynamicIconComponent.Cast(m_wButtonDisableAll.FindHandler(SCR_DynamicIconComponent));


		m_wValidateRepairAllEnabled = ButtonWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_EnabledAddonsPanel.Overlay0.VerticalLayout0.TopBarHeight.TopBar.Content.HorizontalLayout0.m_ValidateRepairAllEnabled"));
		m_ValidateRepairAllEnabledComponent0 = SCR_ModularButtonComponent.Cast(m_wValidateRepairAllEnabled.FindHandler(SCR_ModularButtonComponent));
		m_ValidateRepairAllEnabledComponent1 = SCR_DynamicIconComponent.Cast(m_wValidateRepairAllEnabled.FindHandler(SCR_DynamicIconComponent));


		m_wDeleteAllEnabled = ButtonWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_EnabledAddonsPanel.Overlay0.VerticalLayout0.TopBarHeight.TopBar.Content.HorizontalLayout0.m_DeleteAllEnabled"));
		m_DeleteAllEnabledComponent0 = SCR_ModularButtonComponent.Cast(m_wDeleteAllEnabled.FindHandler(SCR_ModularButtonComponent));
		m_DeleteAllEnabledComponent1 = SCR_DynamicIconComponent.Cast(m_wDeleteAllEnabled.FindHandler(SCR_DynamicIconComponent));

		m_wEnabledAddonsList = VerticalLayoutWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_EnabledAddonsPanel.Overlay0.VerticalLayout0.Content.Scroll.m_EnabledAddonsList"));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetLayout()
	{
		return LAYOUT;
	}
}
