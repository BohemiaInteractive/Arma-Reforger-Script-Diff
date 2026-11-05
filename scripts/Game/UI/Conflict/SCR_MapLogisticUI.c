class SCR_MapLogisticUI : SCR_MapUIElementContainer
{
	[Attribute("{722AE78E8C0D6B71}UI/layouts/Campaign/LogisticBaseElement.layout", params: "layout")]
	protected ResourceName m_sBaseElement;

	[Attribute("{94F1E2223D7E0588}UI/layouts/Campaign/ServiceHint.layout", params: "layout")]
	protected ResourceName m_sServiceHint;

	[Attribute("{4E7B19C2A85D36F0}UI/layouts/Campaign/TransportUnitMapElement.layout", desc: "Map icon of a transport unit.", params: "layout")]
	protected ResourceName m_sTransportUnitElement;

	[Attribute("80", desc: "Size of the transport unit icon on the map [px].", params: "1 inf")]
	protected float m_fTransportUnitIconSize;

	//! Map icons of the local faction transport units, created on map open and removed on map close
	protected ref map<SCR_TransportUnitComponent, SCR_MapUITransportUnit> m_mTransportUnitIcons = new map<SCR_TransportUnitComponent, SCR_MapUITransportUnit>();

	protected SCR_SuppliesTransportSystem m_SuppliesTransportSystem;

	//------------------------------------------------------------------------------------------------
	protected void InitBases()
	{
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (!faction)
			return;

		SCR_MilitaryBaseSystem militaryBaseSystem = SCR_MilitaryBaseSystem.GetInstance();
		array<SCR_MilitaryBaseComponent> bases = {};
		militaryBaseSystem.GetBases(bases);

		SCR_CampaignMilitaryBaseComponent campaignBasebase;
		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBasebase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBasebase || !campaignBasebase.IsInitialized())
				continue;

			// Don't display enemy HQs and established bases which are out of radio range
			if (faction != campaignBasebase.GetFaction() && (campaignBasebase.IsHQ() || (campaignBasebase.GetBuiltByPlayers() && !campaignBasebase.IsHQRadioTrafficPossible(faction))))
				continue;

			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sBaseElement, m_wIconsContainer);
			SCR_CampaignLogisticMapUIBase handler = SCR_CampaignLogisticMapUIBase.Cast(w.FindHandler(SCR_CampaignLogisticMapUIBase));
			if (!handler)
				return;

			handler.SetParent(this);
			handler.InitBase(campaignBasebase);
			m_mIcons.Set(w, handler);
			campaignBasebase.SetBaseUI(handler);

			FrameSlot.SetSizeToContent(w, true);
			FrameSlot.SetAlignment(w, 0.5, 0.5);
		}

		UpdateIcons();
	}

	//------------------------------------------------------------------------------------------------
	protected void InitTransportUnits()
	{
		m_SuppliesTransportSystem = SCR_SuppliesTransportSystem.GetInstance();
		if (!m_SuppliesTransportSystem)
			return;

		m_SuppliesTransportSystem.GetOnTransportUnitAdded().Insert(AddTransportUnitIcon);
		m_SuppliesTransportSystem.GetOnTransportUnitRemoved().Insert(RemoveTransportUnitIcon);

		array<SCR_TransportUnitComponent> transportUnits = {};
		m_SuppliesTransportSystem.GetTransportUnits(transportUnits, SCR_FactionManager.SGetLocalPlayerFaction());
		foreach (SCR_TransportUnitComponent transportUnit : transportUnits)
		{
			AddTransportUnitIcon(transportUnit);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DeinitTransportUnits()
	{
		if (m_SuppliesTransportSystem)
		{
			m_SuppliesTransportSystem.GetOnTransportUnitAdded().Remove(AddTransportUnitIcon);
			m_SuppliesTransportSystem.GetOnTransportUnitRemoved().Remove(RemoveTransportUnitIcon);
		}

		foreach (SCR_TransportUnitComponent transportUnit, SCR_MapUITransportUnit icon : m_mTransportUnitIcons)
		{
			if (icon && icon.GetRoot())
				icon.GetRoot().RemoveFromHierarchy();
		}

		m_mTransportUnitIcons.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void AddTransportUnitIcon(SCR_TransportUnitComponent transportUnit)
	{
		if (!transportUnit || !m_wIconsContainer || m_mTransportUnitIcons.Contains(transportUnit))
			return;

		Faction faction = transportUnit.GetFaction();
		if (!faction || faction != SCR_FactionManager.SGetLocalPlayerFaction())
			return;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sTransportUnitElement, m_wIconsContainer);
		if (!w)
			return;

		SCR_MapUITransportUnit iconHandler = SCR_MapUITransportUnit.Cast(w.FindHandler(SCR_MapUITransportUnit));
		if (!iconHandler)
		{
			w.RemoveFromHierarchy();
			return;
		}

		iconHandler.SetParent(this);
		iconHandler.Init(transportUnit, m_fTransportUnitIconSize);

		FrameSlot.SetSizeToContent(w, true);
		FrameSlot.SetAlignment(w, 0.5, 0.5);

		m_mTransportUnitIcons.Insert(transportUnit, iconHandler);
		UpdateTransportUnitIcon(iconHandler);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveTransportUnitIcon(SCR_TransportUnitComponent transportUnit)
	{
		SCR_MapUITransportUnit icon = m_mTransportUnitIcons.Get(transportUnit);
		if (icon && icon.GetRoot())
			icon.GetRoot().RemoveFromHierarchy();

		m_mTransportUnitIcons.Remove(transportUnit);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTransportUnitIcon(notnull SCR_MapUITransportUnit icon)
	{
		Widget w = icon.GetRoot();
		if (!w)
			return;

		vector position = icon.GetPos();

		// Position not replicated yet
		if (position == vector.Zero)
		{
			w.SetVisible(false);
			return;
		}

		int screenX, screenY;
		m_MapEntity.WorldToScreen(position[0], position[2], screenX, screenY, true);

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		FrameSlot.SetPos(w, workspace.DPIUnscale(screenX), workspace.DPIUnscale(screenY));
		w.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		Widget serviceHint = GetGame().GetWorkspace().CreateWidgets(m_sServiceHint, SCR_MapEntity.GetMapInstance().GetMapMenuRoot());
		if (serviceHint)
		{
			FrameSlot.SetSizeToContent(serviceHint, true);
			serviceHint.SetName("ServiceHintRoot");
			serviceHint.SetVisible(false);
		}

		InitBases();
		InitTransportUnits();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		DeinitTransportUnits();

		super.OnMapClose(config);

		foreach (Widget w, SCR_MapUIElement i : m_mIcons)
		{
			w.RemoveFromHierarchy();
			delete w;
		}

		m_mIcons.Clear();
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);

		// Every frame, so the icons follow map pan and zoom
		foreach (SCR_TransportUnitComponent transportUnit, SCR_MapUITransportUnit icon : m_mTransportUnitIcons)
		{
			if (transportUnit && icon)
				UpdateTransportUnitIcon(icon);
		}

		SCR_CampaignLogisticMapUIBase logisticMapBase;
		foreach (Widget w, SCR_MapUIElement mapUIElement : m_mIcons)
		{
			logisticMapBase = SCR_CampaignLogisticMapUIBase.Cast(mapUIElement);
			if (!logisticMapBase)
				continue;

			if (logisticMapBase.IsSelected() && logisticMapBase.IsOpened())
			{
				GetGame().GetInputManager().ActivateContext("LogisticMapContext");
				break;
			}

		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MapLogisticUI()
	{
		m_mIcons.Clear();
	}
}
