class SCR_MapLogisticUI : SCR_MapUIElementContainer
{
	[Attribute("{722AE78E8C0D6B71}UI/layouts/Campaign/LogisticBaseElement.layout", params: "layout")]
	protected ResourceName m_sBaseElement;

	[Attribute("{94F1E2223D7E0588}UI/layouts/Campaign/ServiceHint.layout", params: "layout")]
	protected ResourceName m_sServiceHint;

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
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
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
		foreach (Widget w, SCR_MapUIElement i : m_mIcons)
		{
			delete w;
		}
	}
}
