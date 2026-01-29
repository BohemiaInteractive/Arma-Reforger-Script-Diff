class SCR_CampaignBuildingServicesEditorUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute("{09DFB36A6C8D45C0}UI/layouts/Editor/GameInfo/GameInfo_CampaignBuilding_ServiceIcon.layout")]
	protected ResourceName m_sServiceIconsGridPrefab;

	[Attribute(DEFAULT_VALUE.ToString(), params: "1 inf 1", desc: "Maximum number of icons in the row")]
	protected int m_iMaxColumns;

	protected ref array<ref Tuple2<EEditableEntityLabel, SCR_ServicePointComponent>> m_aServices = {};

	protected Widget m_wCampaignBuildingServicesRoot;
	protected SCR_MilitaryBaseComponent m_MilitaryBaseComponent;

	protected const int PADDING_LEFT = 5;
	protected const int PADDING_RIGHT = 5;
	protected const int PADDING_TOP = 5;
	protected const int PADDING_BOTTOM = 5;
	protected static const int DEFAULT_VALUE = 7;

	protected ref array<EEditableEntityLabel> m_aAvailableServicesLabel = {};
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wCampaignBuildingServicesRoot = w;

		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;

		// if provider isn't a base, disable the UI.
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(buildingEditorComponent.GetProviderEntity().FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;
		
		SCR_MilitaryBaseComponent base = providerComponent.GetMilitaryBaseComponent();
		if (!base)
		{
			w.SetOpacity(0);
			return;
		}

		GetAllServicesUIInfo();
		SetBaseServices();
		RefreshServiceUI();
		
		base.GetOnServiceRegistered().Insert(SetBaseServices);
		base.GetOnServiceUnregistered().Insert(SetBaseServices);
		base.GetOnServiceUnregistered().Insert(OnServiceUnregistered);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		if (!m_MilitaryBaseComponent)
			return;
		
		m_MilitaryBaseComponent.GetOnServiceUnregistered().Remove(OnServiceUnregistered);
		
		RemoveOnServiceStateChangedEvent();
	}
	
	//------------------------------------------------------------------------------------------------
	// Go through all labels of service and filter out those belonging to a service type. One composition (EditableEntityUIInfo) can holds more then one service.
	protected void FilterServiceLabels(notnull SCR_EditableEntityUIInfo editableUIInfo)
	{
		SCR_EditableEntityCore entityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!entityCore)
			return;
		
		array<EEditableEntityLabel> entityLabels = {};
		array<ref SCR_EditableEntityCampaignBuildingModeLabelData> buildingModeLabelData = {};
		
		editableUIInfo.GetEntityLabels(entityLabels);
		
		// This composition doesn't have a service trait, no need to evaluate it.
		if (!entityLabels.Contains(EEditableEntityLabel.TRAIT_SERVICE))
			return;
		
		entityCore.GetCampaignBuildingModeLabelsData(entityLabels, buildingModeLabelData);
				
		foreach (SCR_EditableEntityCampaignBuildingModeLabelData data : buildingModeLabelData)
		{
			if (!m_aAvailableServicesLabel.Contains(data.GetEntityLabel()))
				m_aAvailableServicesLabel.Insert(data.GetEntityLabel());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GetAllServicesUIInfo()
	{
		SCR_PlacingEditorComponentClass placingPrefabData = SCR_PlacingEditorComponentClass.Cast(SCR_PlacingEditorComponentClass.GetInstance(SCR_PlacingEditorComponent, true));
		if (!placingPrefabData)
			return;

		array<ResourceName> AllPrefabs = {};
		placingPrefabData.GetPrefabs(AllPrefabs);

		foreach (ResourceName prefab : AllPrefabs)
		{
			Resource entityPrefab = Resource.Load(prefab);
			if (!entityPrefab.IsValid())
				continue;

			IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(entityPrefab);
			if (!entitySource)
				continue;

			IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(entitySource);
			if (!editableEntitySource)
				continue;

			SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityComponentClass.GetInfo(editableEntitySource);
			if (!editableUIInfo)
				continue;

			IEntity player = EntityUtils.GetPlayer();
			if (!player)
				continue;
			
			if (editableUIInfo.GetFaction() != SCR_Faction.GetEntityFaction(player))
				continue;

			FilterServiceLabels(editableUIInfo);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetBaseServices(SCR_MilitaryBaseComponent militaryBaseComponent = null, SCR_ServicePointComponent serviceComponent = null)
	{
		SCR_EditableEntityCore entityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!entityCore)
			return;
		
		RemoveOnServiceStateChangedEvent();

		m_MilitaryBaseComponent = militaryBaseComponent;
		
		array<SCR_ServicePointComponent> services = {};
		array<int> allEditorLabels = {};
		array<ref SCR_EditableEntityCampaignBuildingModeLabelData> serviceLabelData = {};
		
		GetProviderServices(services);

		SCR_Enum.GetEnumValues(EEditableEntityLabel, allEditorLabels);

		entityCore.GetCampaignBuildingModeLabelsData(allEditorLabels, serviceLabelData);
		
		m_aServices.Clear();

		foreach (SCR_ServicePointComponent service : services)
		{
			if (service.GetLabel() == EEditableEntityLabel.NONE)
				continue;

			m_aServices.Insert(new Tuple2<EEditableEntityLabel, SCR_ServicePointComponent>(service.GetLabel(), service));
			service.GetOnServiceStateChanged().Insert(RefreshServiceUI);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns all services available with current provider.
	protected void GetProviderServices(out array<SCR_ServicePointComponent> services)
	{
		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;
		
		SCR_MilitaryBaseComponent militaryBaseComponent = buildingEditorComponent.GetProviderComponent().GetMilitaryBaseComponent();
		if (!militaryBaseComponent)
			return;
		
		militaryBaseComponent.GetServices(services);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes the update ui event from all currently available services
	protected void RemoveOnServiceStateChangedEvent()
	{
		array<SCR_ServicePointComponent> services = {};
		GetProviderServices(services);
		
		foreach (SCR_ServicePointComponent service : services)
		{
			service.GetOnServiceStateChanged().Remove(RefreshServiceUI);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnServiceUnregistered(SCR_MilitaryBaseComponent militaryBaseComponent , SCR_ServicePointComponent service)
	{
		RefreshServiceUI();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnServiceChanged(SCR_EServicePointStatus state)
	{
		RefreshServiceUI();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Refresh the service widgets to reflect current status of services in base.
	protected void RefreshServiceUI()
	{
		ClearServicesWidget();
		SetServicesIcon();
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearServicesWidget()
	{
		Widget gridWidget = m_wCampaignBuildingServicesRoot.FindAnyWidget("Grid");
		if (!gridWidget)
			return;

		SCR_WidgetHelper.RemoveAllChildren(gridWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	protected Widget CreateWidget(notnull Widget gridWidget, int serviceId)
	{
		Widget serviceBtn = GetGame().GetWorkspace().CreateWidgets(m_sServiceIconsGridPrefab, gridWidget);
		if (!serviceBtn)
			return null;
		
		int row = Math.Floor(serviceId / m_iMaxColumns);
		int column = serviceId % m_iMaxColumns;
		GridSlot.SetPadding(serviceBtn, PADDING_LEFT, 0, PADDING_RIGHT, 0);
		GridSlot.SetRow(serviceBtn, row);
		GridSlot.SetColumn(serviceBtn, column);
		
		return serviceBtn;
	}

	//------------------------------------------------------------------------------------------------
	//! Go through all available services and set an icon to it.
	protected void SetServicesIcon()
	{
		if (m_iMaxColumns < 1)
			m_iMaxColumns = DEFAULT_VALUE;
				
		Widget serviceBtn;
		Widget gridWidget = m_wCampaignBuildingServicesRoot.FindAnyWidget("Grid");
		if (!gridWidget)
			return;
		
		// Iterate all services, check if they are avialable and if so, set the icon properly.
		for (int i = m_aAvailableServicesLabel.Count() -1; i >= 0; --i)
		{
			serviceBtn = CreateWidget(gridWidget, i);
			
			if (IsAnyServiceBuilt(m_aAvailableServicesLabel[i]))
				serviceBtn.SetOpacity(1);
			else
				serviceBtn.SetOpacity(0.25);

			ImageWidget imgWidget = ImageWidget.Cast(serviceBtn.FindAnyWidget("Image"));
			if (!imgWidget)
				continue;

			SetIcon(imgWidget, m_aAvailableServicesLabel[i]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if any listed service is fully built.
	//! \param[in] label
	//! \return
	bool IsAnyServiceBuilt(EEditableEntityLabel label)
	{		
		foreach (Tuple2<EEditableEntityLabel, SCR_ServicePointComponent> service : m_aServices)
		{
			if (service.param1 != label)
				continue;

			if (service.param2 && service.param2.GetServiceState() == SCR_EServicePointStatus.ONLINE)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the icon of the service.
	//! \param[in] widget
	//! \param[in] serviceLabel
	protected void SetIcon(ImageWidget widget, EEditableEntityLabel serviceLabel)
	{
		if (!m_aAvailableServicesLabel)
			return;
		
		SCR_EditableEntityCore entityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));	
		if (!entityCore)
			return; 
		
		array<ref SCR_EditableEntityCampaignBuildingModeLabelData> buildingModeLabelData = {};	
		entityCore.GetCampaignBuildingModeLabelsData(m_aAvailableServicesLabel, buildingModeLabelData);		
				
		foreach (SCR_EditableEntityCampaignBuildingModeLabelData data : buildingModeLabelData)
		{
			if (data.GetEntityLabel() == serviceLabel)
				data.GetUIInfo().SetIconTo(widget);
		}		
	}
}
