class SCR_CampaignBuildingRefreshUIComponent : ScriptedWidgetComponent
{
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		SCR_ExternalPaginationUIComponent pagUIComp = SCR_ExternalPaginationUIComponent.Cast(w.FindHandler(SCR_ExternalPaginationUIComponent));
		if (!pagUIComp)
			return;

		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;

		IEntity provider = buildingEditorComponent.GetProviderEntity();
		if (!provider)
			return;

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(provider.FindComponent(SCR_ResourceComponent));
		if (!resourceComponent)
			return;

		resourceComponent.TEMP_GetOnInteractorReplicated().Insert(pagUIComp.RefreshPage);

		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;
		
		providerComponent.GetOnCooldownLockUpdated().Insert(pagUIComp.RefreshPage);

		SCR_MilitaryBaseComponent base = providerComponent.GetMilitaryBaseComponent();
		if (!base)
			return;

		base.GetOnServiceRegistered().Insert(pagUIComp.RefreshPage);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_ExternalPaginationUIComponent pagUIComp = SCR_ExternalPaginationUIComponent.Cast(w.FindHandler(SCR_ExternalPaginationUIComponent));
		if (!pagUIComp)
			return;

		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;

		IEntity provider = buildingEditorComponent.GetProviderEntity();
		if (!provider)
			return;

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(provider.FindComponent(SCR_ResourceComponent));
		if (!resourceComponent)
			return;

		resourceComponent.TEMP_GetOnInteractorReplicated().Remove(pagUIComp.RefreshPage);

		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;
		
		providerComponent.GetOnCooldownLockUpdated().Remove(pagUIComp.RefreshPage);

		SCR_MilitaryBaseComponent base = providerComponent.GetMilitaryBaseComponent();
		if (!base)
			return;

		base.GetOnServiceRegistered().Remove(pagUIComp.RefreshPage);
	}
}
