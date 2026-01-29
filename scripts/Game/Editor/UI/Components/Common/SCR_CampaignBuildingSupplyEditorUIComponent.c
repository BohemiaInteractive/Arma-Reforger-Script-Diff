class SCR_CampaignBuildingSupplyEditorUIComponent : SCR_BaseEditorUIComponent
{
	protected SCR_FactionAffiliationComponent m_FactionComponent;
	protected TextWidget m_ProviderName;
	protected TextWidget m_ProviderCallsign;
	protected Widget m_wInGameSupply;
	protected TextWidget m_ProviderSupplyCurrent;
	protected TextWidget m_ProviderSupplyMax;
	protected OverlayWidget m_ProviderIcon;
	
	protected SCR_ResourceComponent	m_ResourceComponent;
	protected SCR_ResourceConsumer m_ResourceConsumer;
	protected ref SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleConsumer;
	protected RplId m_ResourceInventoryPlayerComponentRplId;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_ProviderName = TextWidget.Cast(w.FindAnyWidget("Provider_Name"));
		m_ProviderCallsign = TextWidget.Cast(w.FindAnyWidget("Provider_Callsign"));
		m_ProviderSupplyCurrent = TextWidget.Cast(w.FindAnyWidget("Supply_Value_Current"));
		m_wInGameSupply = w.FindAnyWidget("Supply_InGame_Supply");
		m_ProviderSupplyMax = TextWidget.Cast(w.FindAnyWidget("Supply_Value_Max"));
		m_ProviderIcon = OverlayWidget.Cast(w.FindAnyWidget("Provider_Icon_Overlay"));
		
		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;
		
		IEntity targetEntity = buildingEditorComponent.GetProviderEntity();
		if (!targetEntity)
			return;		
		
		m_FactionComponent = buildingEditorComponent.GetProviderFactionComponent();
		if (!m_FactionComponent)
			return;
		
		SetSourceIcon(targetEntity);
		SetProviderName(targetEntity);
		
		if (!buildingEditorComponent.GetProviderResourceComponent(m_ResourceComponent))
			return;
		
		if (!m_ResourceComponent || !m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES, m_ResourceConsumer))
			return;
		
		m_ResourceInventoryPlayerComponentRplId = Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent)));
		m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandle(m_ResourceConsumer, m_ResourceInventoryPlayerComponentRplId);
		
		m_ResourceComponent.TEMP_GetOnInteractorReplicated().Insert(UpdateResources);
		// Update once at the beginning and then every time the supply value has changed. 
		UpdateResources();
	}	

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_ResourceComponent)
			m_ResourceComponent.TEMP_GetOnInteractorReplicated().Remove(UpdateResources);
		
		m_ResourceSubscriptionHandleConsumer = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetSourceIcon(IEntity targetEntity)
	{
		if (!m_ProviderIcon)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		Color factionColor;
		// We need to check both here, as vehicle has not set AffiliatedFaction if it is empty, to prevent AI to shoot at the empty vehicle. 
		Faction faction = m_FactionComponent.GetAffiliatedFaction();
		if (!faction)
			faction = m_FactionComponent.GetDefaultAffiliatedFaction();
		
		if (faction)
			factionColor = GetColorForFaction(faction.GetFactionKey());

		//This marker thing should be converted into more sandbox solution later.
		SCR_MilitarySymbolUIComponent m_SymbolUI = SCR_MilitarySymbolUIComponent.Cast(m_ProviderIcon.FindHandler(SCR_MilitarySymbolUIComponent));
		SCR_MilitarySymbol baseIcon = new SCR_MilitarySymbol();
		
		switch(faction.GetFactionKey())
		{
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.INDFOR):
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.INDFOR);
				break;
			}

			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR):
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.OPFOR);
				break;
			}

			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR):
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.BLUFOR);				
				break;
			}

			case "Unknown":
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.UNKNOWN);
				break;
			}
		}
		
		if (Vehicle.Cast(m_FactionComponent.GetOwner()))
			baseIcon.SetIcons(EMilitarySymbolIcon.MOTORIZED);

		baseIcon.SetDimension(2);
		m_ProviderIcon.SetColor(factionColor);
		m_SymbolUI.Update(baseIcon);
	}
	
	//------------------------------------------------------------------------------
	protected Color GetColorForFaction(string factionKey)
	{
		FactionManager fm = GetGame().GetFactionManager();
		if (!fm)
			return null;

		Faction faction = fm.GetFactionByKey(factionKey);
		if (!faction)
			return null;

		return faction.GetFactionColor();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateResources()
	{
		if (!m_ResourceComponent)
			return;
		
		SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		
		if (!consumer)
			return;
		
		if (m_ProviderSupplyCurrent)
			m_ProviderSupplyCurrent.SetText(consumer.GetAggregatedResourceValue().ToString());

		if (m_ProviderSupplyMax)
			m_ProviderSupplyMax.SetText(consumer.GetAggregatedMaxResourceValue().ToString());
		
		// Visualize supply state 
		if (consumer.GetAggregatedResourceValue() == 0)
			m_wInGameSupply.SetColor(UIColors.WARNING);
		else
			m_wInGameSupply.SetColor(Color.FromInt(Color.WHITE));
	}
	
	//------------------------------------------------------------------------------------------------
	[Obsolete("SCR_CampaignBuildingSupplyEditorUIComponent.UpdateResources() should be used instead.")]
	protected void UpdateSupply()
	{
		if (!m_ResourceComponent)
			return;
		
		SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		if (!consumer)
			return;
		
		if (m_ProviderSupplyCurrent)
			m_ProviderSupplyCurrent.SetText(consumer.GetAggregatedResourceValue().ToString());

		if (m_ProviderSupplyMax)
			m_ProviderSupplyMax.SetText(consumer.GetAggregatedMaxResourceValue().ToString());
		
		// Visualize supply state 
		if (consumer.GetAggregatedResourceValue() == 0)
			m_wInGameSupply.SetColor(UIColors.WARNING);
		else
			m_wInGameSupply.SetColor(Color.FromInt(Color.WHITE));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetProviderName(IEntity targetEntity)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(targetEntity.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;

		string providerName = providerComponent.GetProviderDisplayName();
		string callsignName;

		// Campaign behavior
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign)
		{
			SCR_CampaignMilitaryBaseComponent targetBase = providerComponent.GetCampaignMilitaryBaseComponent();
			if (targetBase)
			{
				callsignName = targetBase.GetCallsignDisplayName();
				providerName = targetBase.GetBaseName();
			}
		}
		else
		{
			SCR_MilitaryBaseComponent targetBase = providerComponent.GetMilitaryBaseComponent();
			if (targetBase)
				callsignName = targetBase.GetCallsignDisplayName();
		}

		m_ProviderCallsign.SetText(callsignName);
		m_ProviderName.SetText(providerName);
	}
}
