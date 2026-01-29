class SCR_SuppliesTransportSystemUI : SCR_ScriptedWidgetComponent
{
	[Attribute("{4093FEFCBC51D74A}UI/layouts/Campaign/TransportUnit.layout", desc: "Trasnport unit layout path.")]
	protected ResourceName m_sTransportUnitLayoutResource;

	[Attribute("Labels")]
	protected string m_sLabelsWidgetName;

	[Attribute("TransportUnitsContainer")]
	protected string m_sTransportUnitsContainerWidgetName;

	[Attribute("NoTransportUnitInfo")]
	protected string m_sNoTransportUnitInfoWidgetName;

	protected Widget m_wLabels;
	protected Widget m_wTransportUnitsContainer;
	protected Widget m_wNoTransportUnitInfo;

	protected SCR_SuppliesTransportSystem m_SuppliesTransportSystem;

	protected ref map<SCR_TransportUnitComponent, SCR_TransportUnitUI> m_mTransportUnits = new map<SCR_TransportUnitComponent, SCR_TransportUnitUI>();

	//------------------------------------------------------------------------------------------------
	void Init()
	{
		if (!m_SuppliesTransportSystem)
			return;

		m_SuppliesTransportSystem.GetOnTransportUnitAdded().Insert(CreateTransportUnit);
		m_SuppliesTransportSystem.GetOnTransportUnitRemoved().Insert(RemoveTransportUnit);

		array<SCR_TransportUnitComponent> transportUnits = {};
		m_SuppliesTransportSystem.GetTransportUnits(transportUnits, SCR_FactionManager.SGetLocalPlayerFaction());

		bool isTransportUnitsEmpty = transportUnits.IsEmpty();
		if (m_wNoTransportUnitInfo)
			m_wNoTransportUnitInfo.SetVisible(isTransportUnitsEmpty);

		if (m_wLabels)
			m_wLabels.SetVisible(!isTransportUnitsEmpty);

		foreach (SCR_TransportUnitComponent theTransportUnit : transportUnits)
		{
			CreateTransportUnit(theTransportUnit);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Deinit()
	{
		if (!m_SuppliesTransportSystem)
			return;

		array<SCR_TransportUnitComponent> transportUnits = {};
		m_SuppliesTransportSystem.GetTransportUnits(transportUnits, SCR_FactionManager.SGetLocalPlayerFaction());
		for (int i = transportUnits.Count() - 1; i >= 0; i--)
		{
			RemoveTransportUnit(transportUnits[i]);
		}

		m_SuppliesTransportSystem.GetOnTransportUnitAdded().Remove(CreateTransportUnit);
		m_SuppliesTransportSystem.GetOnTransportUnitRemoved().Remove(RemoveTransportUnit);
	}

	//------------------------------------------------------------------------------------------------
	void ToggleVisibility(bool isVisible)
	{
		if (isVisible && !m_mTransportUnits.IsEmpty())
			m_mTransportUnits.GetElement(0).FocusButton();
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateTransportUnit(notnull SCR_TransportUnitComponent transportUnit)
	{
		if (!m_wTransportUnitsContainer)
			return;

		if (transportUnit.GetFaction() != SCR_FactionManager.SGetLocalPlayerFaction())
			return;

		Widget transportUnitWidget = GetGame().GetWorkspace().CreateWidgets(m_sTransportUnitLayoutResource, m_wTransportUnitsContainer);
		if (!transportUnitWidget)
			return;

		SCR_TransportUnitUI transportUnitUI = SCR_TransportUnitUI.Cast(transportUnitWidget.FindHandler(SCR_TransportUnitUI));
		transportUnitUI.Init(transportUnit);
		m_mTransportUnits.Insert(transportUnit, transportUnitUI);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveTransportUnit(SCR_TransportUnitComponent transportUnit)
	{
		SCR_TransportUnitUI transportUnitUI = m_mTransportUnits.Get(transportUnit);
		if (!transportUnitUI)
			return;

		transportUnitUI.Deinit();
		transportUnitUI.GetRootWidget().RemoveFromHierarchy();
		m_mTransportUnits.Remove(transportUnit);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
			return;

		super.HandlerAttached(w);

		m_wLabels = w.FindAnyWidget(m_sLabelsWidgetName);

		m_wTransportUnitsContainer = w.FindAnyWidget(m_sTransportUnitsContainerWidgetName);
		if (!m_wTransportUnitsContainer)
			return;

		SCR_WidgetHelper.RemoveAllChildren(m_wTransportUnitsContainer);

		m_wNoTransportUnitInfo = w.FindAnyWidget(m_sNoTransportUnitInfoWidgetName);

		m_SuppliesTransportSystem = SCR_SuppliesTransportSystem.GetInstance();
	}
}
