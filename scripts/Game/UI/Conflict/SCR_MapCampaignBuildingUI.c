class SCR_MapCampaignBuildingUI : SCR_MapUIBaseComponent
{
	protected SCR_MapCampaignUI m_MapCampaignUI
	protected SCR_CampaignBuildingProviderComponent m_TemporaryProviderComponent;

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
		int providerCount = baseUI.GetBase().GetBuildingProviders(campaignBuildingProvides);
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

		int playerID = SCR_PlayerController.GetLocalPlayerId();
		m_TemporaryProviderComponent.RequestBuildingMode(playerID, true);

		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		editorManager.GetOnClosed().Insert(OnModeClosed);
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
	//! \return local Editor Manager
	protected SCR_EditorManagerEntity GetEditorManager()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		return core.GetEditorManager();
	}
}
