class SCR_MapUISpawnPoint : SCR_MapUIElement
{
	[Attribute("Select")]
	protected string m_sSelection;
	[Attribute("Base_Main")]
	protected string m_sSpawnPoint;

	protected SCR_SpawnPoint m_SpawnPoint;
	protected RplId m_SpawnPointId;
	protected string m_sFactionKey;
	protected TextWidget m_wSpawnPointName;
	protected Widget m_wImageOverlay;
	protected OverlayWidget m_wSymbolOverlay;
	protected SCR_MilitarySymbolUIComponent m_MilitarySymbolComponent;
	protected ButtonWidget m_wButton;
	protected SizeLayoutWidget m_wSizeLayout;
	protected Widget m_wSupplies;
	protected RichTextWidget m_wSuppliesText
	
	protected SCR_ResourceComponent	m_ResourceComponent;
	protected SCR_ResourceConsumer m_ResourceConsumer;
	protected ref SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleConsumer;
	protected RplId m_ResourceInventoryPlayerComponentRplId;

	//------------------------------------------------------------------------------
	void Init(notnull SCR_SpawnPoint spawnPoint)
	{
		m_SpawnPoint = spawnPoint;
		m_SpawnPointId = spawnPoint.GetRplId();
		m_sFactionKey = spawnPoint.GetFactionKey();
		// TODO@AS: Api?
		m_wSpawnPointName.SetText(spawnPoint.GetSpawnPointName());
		UpdateIcon();

		if (!m_wSuppliesText)
			return;

		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(spawnPoint);
		
		if (!m_ResourceComponent)
			return;

		m_ResourceConsumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

		if (!m_ResourceConsumer)
			return;

		m_ResourceInventoryPlayerComponentRplId = Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent)));
		m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandle(m_ResourceConsumer, m_ResourceInventoryPlayerComponentRplId);
		
		m_ResourceComponent.TEMP_GetOnInteractorReplicated().Insert(UpdateResources);
		UpdateResources();
	}

	//------------------------------------------------------------------------------------------------
	override void SetName(string name)
	{
		if (m_wSpawnPointName)
			m_wSpawnPointName.SetText(name);
	}

	//------------------------------------------------------------------------------------------------
	override vector GetPos()
	{
		return m_SpawnPoint.GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateResources()
	{
		if (!m_ResourceComponent || !m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES, m_ResourceConsumer))
			return;

		m_wSuppliesText.SetText(string.ToString(m_ResourceConsumer.GetAggregatedResourceValue()));
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------
	void UpdateIcon()
	{
		SCR_MilitarySymbol symbol = new SCR_MilitarySymbol();
		
		SCR_GroupIdentityCore core = SCR_GroupIdentityCore.Cast(SCR_GroupIdentityCore.GetInstance(SCR_GroupIdentityCore));
		if (!core)
			return;
		
		SCR_Faction faction = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey(m_sFactionKey));
		if (faction)
		{
			SCR_MilitarySymbolRuleSet ruleSet = core.GetSymbolRuleSet();
			ruleSet.UpdateSymbol(symbol, faction);
		}
		else
		{
			symbol.SetIdentity(EMilitarySymbolIdentity.UNKNOWN);
		}
		
		//Selection visuals 
		string selection;
		string highlight;
		switch (symbol.GetIdentity())
		{
			case EMilitarySymbolIdentity.INDFOR:
			{
				selection = "Neutral_Select";
				highlight = "Neutral_Focus";
				break;
			}
			case EMilitarySymbolIdentity.OPFOR:
			{
				selection = "Hostile_Select";
				highlight = "Hostile_Focus";
				break;
			}
			case EMilitarySymbolIdentity.BLUFOR:
			{
				selection = "Friend_Select";
				highlight = "Friend_Focus";
				break;
			}
			case EMilitarySymbolIdentity.UNKNOWN:
			{
				selection = "Unknown_Select";
				highlight = "Unknown_Focus";
				break;
			}
		}
		m_bVisible = true;
		
		m_wHighlightImg.LoadImageFromSet(0, m_sImageSetARO, highlight);
		m_wSelectImg.LoadImageFromSet(0, m_sImageSetARO, selection);
		
		if (SCR_PlayerSpawnPoint.Cast(m_SpawnPoint))
			symbol.SetIcons(EMilitarySymbolIcon.RELAY);
		else
			symbol.SetIcons(EMilitarySymbolIcon.RESPAWN);
		
		m_wSymbolOverlay.SetColor(GetColorForFaction(m_sFactionKey));
		if (m_wGradient)
			m_wGradient.SetColor(GetColorForFaction(m_sFactionKey));

		if (faction)
			m_MilitarySymbolComponent.Update(symbol);
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wImageOverlay = w.FindAnyWidget("IconOverlay");
		m_wSizeLayout = SizeLayoutWidget.Cast(w.FindAnyWidget("SizeLayout"));
		m_wSpawnPointName = TextWidget.Cast(w.FindAnyWidget("Name"));
		m_wSymbolOverlay = OverlayWidget.Cast(m_wImageOverlay.FindWidget("Symbol"));

		if (!m_wSymbolOverlay)
			return;
		
		m_MilitarySymbolComponent = SCR_MilitarySymbolUIComponent.Cast(m_wSymbolOverlay.FindHandler(SCR_MilitarySymbolUIComponent));
		
		m_wButton = ButtonWidget.Cast(w.FindAnyWidget("Button"));

		if (m_wButton)
			m_wButton.SetEnabled(false);

		m_wSupplies = w.FindAnyWidget("w_Supplies");
		
		if (m_wSupplies)
			m_wSuppliesText = RichTextWidget.Cast(m_wSupplies.FindAnyWidget("SuppliesText"));
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		if (m_ResourceComponent)
			m_ResourceComponent.TEMP_GetOnInteractorReplicated().Remove(UpdateResources);
		
		m_ResourceSubscriptionHandleConsumer = null;
	}

	//------------------------------------------------------------------------------
	override void SelectIcon(bool invoke = true)
	{
		if (!m_wSelectImg)
			return;

		if (s_SelectedElement && s_SelectedElement != this)
			s_SelectedElement.Select(false);

		Select();

		m_wSelectImg.SetVisible(true);
		if (m_wGradient)
			m_wGradient.SetVisible(true);

		if (m_bIsSelected && invoke)
			m_Parent.OnSpawnPointSelected(m_SpawnPointId);
	}

	// todo@lk: plug this somewhere later
	// protected void OnSelected(SCR_SpawnPoint sp)
	// {
	// 	if (!sp)
	// 		return;
	// 	if (sp == m_SpawnPoint)
	// 	{
	// 		AnimExpand();
	// 		m_wRoot.SetZOrder(1);
	// 		m_wSelectImg.SetVisible(true);
	// 		if (m_wGradient)
	// 			m_wGradient.SetVisible(true);
	// 	}
	// 	else
	// 	{
	// 		AnimCollapse();
	// 		m_wRoot.SetZOrder(0);
	// 		m_wSelectImg.SetVisible(false);
	// 		if (m_wGradient)
	// 			m_wGradient.SetVisible(false);
	// 	}
	// }

	//------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_wSizeLayout && w == m_wSizeLayout && m_wButton)
			m_wButton.SetEnabled(true);
		
		GetGame().GetWorkspace().SetFocusedWidget(w);
		super.OnMouseEnter(w, x, y);
		m_wRoot.SetZOrder(1);
		m_wHighlightImg.SetVisible(true);

		if (m_wGradient)
			m_wGradient.SetVisible(true);

		if (!m_bIsSelected)
			AnimExpand();

		return false;
	}

	//------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		m_wRoot.SetZOrder(0);
		m_wHighlightImg.SetVisible(false);
		if (!m_bIsSelected && m_wGradient)
			m_wGradient.SetVisible(false);
		if (!m_bIsSelected)
		{
			AnimCollapse();
		}
		
		if (RenderTargetWidget.Cast(enterW) && m_wButton.IsEnabled())
			m_wButton.SetEnabled(false);

		return false;
	}

	override RplId GetSpawnPointId()
	{
		return m_SpawnPointId;
	}
};