class SCR_BuildModeShowHideOnSupplyEnabledUIComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("1", desc: "Hide when supplies are disabled and show when supplies enabled if true otherwise it will do the opposite")]
	protected bool m_bShowWhenSuppliesEnabled;
	
	protected SCR_CampaignBuildingEditorComponent m_CampaignBuildingEditorComponent;
	protected SCR_ResourceComponent m_ResourceComponent;
	
	//------------------------------------------------------------------------------------------------
	protected void OnResourceTypeEnabledChanged(SCR_ResourceComponent resourceComponent, array<EResourceType> disabledResourceTypes)
	{
		bool suppliesEnabled;
		
		//~ Get setting from ResourceComponent
		if (m_ResourceComponent)
			suppliesEnabled = m_ResourceComponent.IsResourceTypeEnabled();
		//~ No resourceComponent so get settings from Gamemode
		else 
			suppliesEnabled = SCR_ResourceSystemHelper.IsGlobalResourceTypeEnabled();
		
		m_wRoot.SetVisible(suppliesEnabled == m_bShowWhenSuppliesEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnProviderChanged(SCR_CampaignBuildingProviderComponent provider)
	{
		if (m_ResourceComponent)
			m_ResourceComponent.GetOnResourceTypeEnabledChanged().Remove(OnResourceTypeEnabledChanged);
		
		m_ResourceComponent = provider.GetResourceComponent();
		if (m_ResourceComponent)
			m_ResourceComponent.GetOnResourceTypeEnabledChanged().Insert(OnResourceTypeEnabledChanged);
			
		//~ Refresh
		OnResourceTypeEnabledChanged(null, null);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnGlobalResourceEnabledChanged(array<EResourceType> disabledResourceTypes)
	{
		//~ Refresh
		OnResourceTypeEnabledChanged(null, disabledResourceTypes);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		if (!GetGame().InPlayMode())
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnResourceTypeEnabledChanged().Insert(OnGlobalResourceEnabledChanged);
		
		m_CampaignBuildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (m_CampaignBuildingEditorComponent)
		{
			m_CampaignBuildingEditorComponent.GetOnProviderChanged().Insert(OnProviderChanged);
		
			IEntity provider = m_CampaignBuildingEditorComponent.GetProviderEntity(true);
			if (provider)
			{
				m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(provider);
				if (m_ResourceComponent)
					m_ResourceComponent.GetOnResourceTypeEnabledChanged().Insert(OnResourceTypeEnabledChanged);
			}
		}
		
		OnResourceTypeEnabledChanged(null, null);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (SCR_Global.IsEditMode())
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnResourceTypeEnabledChanged().Remove(OnGlobalResourceEnabledChanged);
		
		if (m_CampaignBuildingEditorComponent)
			m_CampaignBuildingEditorComponent.GetOnProviderChanged().Remove(OnProviderChanged);
		
		if (m_ResourceComponent)
			m_ResourceComponent.GetOnResourceTypeEnabledChanged().Remove(OnResourceTypeEnabledChanged);
	}
}
