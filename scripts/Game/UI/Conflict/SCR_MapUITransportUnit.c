//! Map icon of a transport unit (AI logistics group) in the logistics station map.
//! Clicking it highlights the unit in the transport units list.
class SCR_MapUITransportUnit : SCR_MapUIElement
{
	protected SCR_TransportUnitComponent m_TransportUnit;

	protected SizeLayoutWidget m_wSizeLayout;
	protected OverlayWidget m_wSymbolOverlay;
	protected TextWidget m_wName;
	protected SCR_MilitarySymbolUIComponent m_MilitarySymbolComponent;

	//------------------------------------------------------------------------------------------------
	//! \param[in] transportUnit Transport unit shown by this icon
	//! \param[in] size Icon size [px]
	void Init(notnull SCR_TransportUnitComponent transportUnit, float size)
	{
		m_TransportUnit = transportUnit;

		if (m_wSizeLayout)
		{
			m_wSizeLayout.SetWidthOverride(size);
			m_wSizeLayout.SetHeightOverride(size);
		}

		if (m_wName)
			m_wName.SetText(transportUnit.GetName());

		UpdateIcon();
	}

	//------------------------------------------------------------------------------------------------
	SCR_TransportUnitComponent GetTransportUnit()
	{
		return m_TransportUnit;
	}

	//------------------------------------------------------------------------------------------------
	//! \return World position of the transport unit, vector.Zero when not known yet
	override vector GetPos()
	{
		if (!m_TransportUnit)
			return vector.Zero;

		return m_TransportUnit.GetMapPosition();
	}

	//------------------------------------------------------------------------------------------------
	//! Shows the group military symbol in the faction color
	protected void UpdateIcon()
	{
		SCR_AIGroup group = m_TransportUnit.GetAIGroup();
		if (!group)
			return;

		SCR_GroupIdentityComponent groupIdentity = SCR_GroupIdentityComponent.Cast(group.FindComponent(SCR_GroupIdentityComponent));
		if (!groupIdentity)
			return;

		SCR_MilitarySymbol symbol = groupIdentity.GetMilitarySymbol();
		if (!symbol)
			return;

		m_bVisible = true;

		string highlight;
		switch (symbol.GetIdentity())
		{
			case EMilitarySymbolIdentity.INDFOR:
			{
				highlight = "Neutral_Focus";
				break;
			}
			case EMilitarySymbolIdentity.OPFOR:
			{
				highlight = "Hostile_Focus";
				break;
			}
			case EMilitarySymbolIdentity.BLUFOR:
			{
				highlight = "Friend_Focus";
				break;
			}
			default:
			{
				highlight = "Unknown_Focus";
				break;
			}
		}

		if (m_wHighlightImg)
			m_wHighlightImg.LoadImageFromSet(0, m_sImageSetARO, highlight);

		Faction faction = m_TransportUnit.GetFaction();
		if (faction)
		{
			if (m_wSymbolOverlay)
				m_wSymbolOverlay.SetColor(faction.GetFactionColor());

			if (m_wGradient)
				m_wGradient.SetColor(faction.GetFactionColor());
		}

		if (m_MilitarySymbolComponent)
			m_MilitarySymbolComponent.Update(symbol);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetHovered(bool hovered)
	{
		if (m_wHighlightImg)
			m_wHighlightImg.SetVisible(hovered);

		if (m_wGradient)
			m_wGradient.SetVisible(hovered);

		if (m_wName)
			m_wName.SetVisible(hovered);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		SetHovered(true);

		// Draw the hovered icon (and its name) above the other icons
		m_wRoot.SetZOrder(2);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		SetHovered(false);
		m_wRoot.SetZOrder(1);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_TransportUnit || !m_bVisible)
			return false;

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return false;

		SCR_MapSuppliesTransportSystemUI transportSystemUI = SCR_MapSuppliesTransportSystemUI.Cast(mapEntity.GetMapUIComponent(SCR_MapSuppliesTransportSystemUI));
		if (transportSystemUI)
			transportSystemUI.HighlightTransportUnit(m_TransportUnit);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wSizeLayout = SizeLayoutWidget.Cast(w.FindAnyWidget("SizeLayout"));
		m_wName = TextWidget.Cast(w.FindAnyWidget("Name"));

		m_wSymbolOverlay = OverlayWidget.Cast(w.FindAnyWidget("Symbol"));
		if (m_wSymbolOverlay)
			m_MilitarySymbolComponent = SCR_MilitarySymbolUIComponent.Cast(m_wSymbolOverlay.FindHandler(SCR_MilitarySymbolUIComponent));
	}
}
