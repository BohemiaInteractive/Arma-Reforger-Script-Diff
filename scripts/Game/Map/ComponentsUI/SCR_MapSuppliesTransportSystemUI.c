class SCR_MapSuppliesTransportSystemUI : SCR_MapUIBaseComponent
{
	[Attribute("MapSuppliesTransportSystem", desc: "Root frame widget name.")]
	protected string m_sRootWidgetName;

	[Attribute("vehicles", desc: "Toolmenu imageset quad name.")]
	protected string m_sToolMenuIconName;

	[Attribute("{B7B891F635B40E1E}UI/layouts/Campaign/SuppliesTransportSystem.layout", desc: "Supplies Transport System layout path.")]
	protected ResourceName m_sSuppliesTransportSystemLayout;

	protected Widget m_wSuppliesTransportSystemFrame;

	protected SCR_MapToolMenuUI m_ToolMenu;
	protected SCR_MapToolEntry m_ToolMenuEntry;

	protected SCR_SuppliesTransportSystemUI m_SuppliesTransportSystemUI;

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		m_ToolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (m_ToolMenu)
		{
			m_ToolMenuEntry = m_ToolMenu.RegisterToolMenuEntry(SCR_MapToolMenuUI.s_sToolMenuIcons, m_sToolMenuIconName, 0, m_bIsExclusive);
			m_ToolMenuEntry.m_OnClick.Insert(ToggleVisible);
			m_ToolMenuEntry.GetOnDisableMapUIInvoker().Insert(Disable);
			m_ToolMenuEntry.SetEnabled(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		m_wSuppliesTransportSystemFrame = m_RootWidget.FindAnyWidget(m_sRootWidgetName);
		if (!m_wSuppliesTransportSystemFrame)
			return;

		if (!m_wSuppliesTransportSystemFrame.GetChildren())
		{
			Widget suppliesTransportSystemWidget = GetGame().GetWorkspace().CreateWidgets(m_sSuppliesTransportSystemLayout, m_wSuppliesTransportSystemFrame);
			if (!suppliesTransportSystemWidget)
				return;

			m_SuppliesTransportSystemUI = SCR_SuppliesTransportSystemUI.Cast(suppliesTransportSystemWidget.FindHandler(SCR_SuppliesTransportSystemUI));
			if (!m_SuppliesTransportSystemUI)
				return;

			m_SuppliesTransportSystemUI.Init();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		super.OnMapClose(config);

		if (m_SuppliesTransportSystemUI)
			m_SuppliesTransportSystemUI.Deinit();
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleVisible()
	{
		if (!m_wSuppliesTransportSystemFrame)
			return;

		bool visible = m_wSuppliesTransportSystemFrame.IsVisible();
		m_wSuppliesTransportSystemFrame.SetVisible(!visible);

		if (m_SuppliesTransportSystemUI)
			m_SuppliesTransportSystemUI.ToggleVisibility(!visible);

		if (m_ToolMenuEntry)
			m_ToolMenuEntry.SetActive(!visible);
	}

	//------------------------------------------------------------------------------------------------
	void Disable()
	{
		m_ToolMenuEntry.SetActive(false);
		m_wSuppliesTransportSystemFrame.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	bool IsVisible()
	{
		return m_wSuppliesTransportSystemFrame && m_wSuppliesTransportSystemFrame.IsVisible();
	}

	//------------------------------------------------------------------------------------------------
	override event void HandlerDeattached(Widget w)
	{
		if (m_ToolMenuEntry)
			m_ToolMenuEntry.GetOnDisableMapUIInvoker().Remove(Disable);
	}
}
