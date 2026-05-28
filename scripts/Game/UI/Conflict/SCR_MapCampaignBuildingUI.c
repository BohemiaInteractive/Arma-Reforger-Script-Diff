class SCR_MapCampaignBuildingUI : SCR_MapUIBaseComponent
{
	protected SCR_MapCampaignUI m_MapCampaignUI
	protected SCR_CampaignBuildingProviderComponent m_TemporaryProviderComponent;
	protected SCR_CampaignMilitaryBaseComponent m_Base;

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();

		m_MapCampaignUI = SCR_MapCampaignUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapCampaignUI));
		if (!m_MapCampaignUI)
			return;

		m_MapCampaignUI.GetOnBasesInited().Insert(OnBasesInited);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBasesInited()
	{
		// register to base OnClick Event
		array<SCR_CampaignMapUIBase> bases = {};
		int count = m_MapCampaignUI.GetBases(bases);
		for (int i = 0; i < count; i++)
		{
			bases[i].m_OnClick.Insert(OnBaseClicked);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseClicked(SCR_CampaignMapUIBase baseUI)
	{
		array<SCR_CampaignBuildingProviderComponent> campaignBuildingProvides = {};
		m_Base = baseUI.GetBase();

		if (!m_Base)
			return;

		int providerCount = m_Base.GetBuildingProviders(campaignBuildingProvides);
		m_TemporaryProviderComponent = null;
		if (providerCount > 0)
		{
			for (int i = 0; i < providerCount; i++)
			{
				if (campaignBuildingProvides[i].IsMasterProvider())
				{
					m_TemporaryProviderComponent = campaignBuildingProvides[i];
					break;
				}
			}
		}

		if (!m_TemporaryProviderComponent)
			return;

		IEntity playerEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!playerEntity)
			return;

		// check player faction
		if (!m_TemporaryProviderComponent.IsEntityFactionSame(playerEntity, m_TemporaryProviderComponent.GetOwner()))
			return;

		m_TemporaryProviderComponent.SetUseAllAvailableProvidersByPlayer(true);

		SCR_CampaignBuildingBudgetToEvaluateData aiBudgetData = m_TemporaryProviderComponent.GetBudgetData(EEditableEntityBudget.AI);
		aiBudgetData.SetShowBudgetInUI(true);

		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		editorManager.GetOnModeAdd().Insert(OnModeAdded);
		editorManager.GetOnClosed().Insert(OnModeClosed);

		int playerID = SCR_PlayerController.GetLocalPlayerId();
		m_TemporaryProviderComponent.RequestEnterBuildingMode(playerID, true, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCameraCreate(SCR_ManualCamera manualCamera)
	{
		SCR_CampaignBuildingCameraEditorComponent editorCameraManager = GetCampaignBuildingCameraEditorComponent();
		if (!editorCameraManager)
			return;

		editorCameraManager.GetOnCameraCreate().Remove(OnCameraCreate);

		// set the camera to the base position
		if (m_Base)
			editorCameraManager.SetPreActivateCameraPosition(m_Base.GetOwner().GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnModeAdded(SCR_EditorModeEntity modeEntity)
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		editorManager.GetOnModeAdd().Remove(OnModeAdded);

		SCR_CampaignBuildingCameraEditorComponent editorCameraManager = GetCampaignBuildingCameraEditorComponent();
		if (!editorCameraManager)
			return;

		editorCameraManager.GetOnCameraCreate().Insert(OnCameraCreate);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnModeClosed()
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		editorManager.GetOnClosed().Remove(OnModeClosed);

		if (!m_TemporaryProviderComponent)
			return;

		m_TemporaryProviderComponent.SetUseAllAvailableProvidersByPlayer(false);

		SCR_CampaignBuildingBudgetToEvaluateData aiBudgetData = m_TemporaryProviderComponent.GetBudgetData(EEditableEntityBudget.AI);
		aiBudgetData.SetShowBudgetInUI(false);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CampaignBuildingCameraEditorComponent GetCampaignBuildingCameraEditorComponent()
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return null;

		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!modeEntity)
			return null;

		return SCR_CampaignBuildingCameraEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingCameraEditorComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! \return local Editor Manager
	protected SCR_EditorManagerEntity GetEditorManager()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		return core.GetEditorManager();
	}
}
