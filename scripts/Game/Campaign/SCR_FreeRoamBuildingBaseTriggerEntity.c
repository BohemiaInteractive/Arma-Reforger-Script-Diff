class SCR_FreeRoamBuildingBaseTriggerEntityClass : ScriptedGameTriggerEntityClass
{
};

class SCR_FreeRoamBuildingBaseTriggerEntity : ScriptedGameTriggerEntity
{
	protected SCR_CampaignBuildingManagerComponent m_BuildingManagerComponent;
	protected SCR_CampaignBuildingProviderComponent m_ProviderComponent
	protected bool m_bUserActionActivation;

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		m_BuildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));

		IEntity provider = GetParent();
		if (!provider)
			return;

		m_ProviderComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (m_ProviderComponent)
			m_bUserActionActivation = m_ProviderComponent.GetUserActionInitOnly();
	}

	//------------------------------------------------------------------------------------------------
	//! Get Editor mode
	protected SCR_EditorManagerEntity GetEditorManager()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		return core.GetEditorManager();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_FreeRoamBuildingBaseTriggerEntity(IEntitySource src, IEntity parent)
	{
	}
};
