class SCR_BuildingCampaignMilitaryBaseTaskEntityClass : SCR_CampaignMilitaryBaseTaskEntityClass
{
}

class SCR_BuildingCampaignMilitaryBaseTaskEntity : SCR_CampaignMilitaryBaseTaskEntity
{
	protected RplComponent m_RplComponent;

	//------------------------------------------------------------------------------------------------
	protected void OnCompositionUnregistered(SCR_CampaignBuildingCompositionComponent composition)
	{
		if (!m_MilitaryBase)
			return;

		bool areAllSpawned = AreAllCompositionsSpawned();

		// all buildable composition was obviously destroyed/deleted - we are canceling the task
		if (!areAllSpawned)
			return;

		m_TaskSystem.SetTaskState(this, SCR_ETaskState.CANCELLED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnyCompositionSpawned()
	{
		if (!m_MilitaryBase)
			return;

		bool areAllSpawned = AreAllCompositionsSpawned();
		if (!areAllSpawned)
			return;

		m_TaskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTargetBaseCaptured(SCR_MilitaryBaseComponent base, Faction faction)
	{
		if (!m_MilitaryBase || base != m_MilitaryBase)
			return;

		m_TaskSystem.SetTaskState(this, SCR_ETaskState.CANCELLED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	protected bool AreAllCompositionsSpawned()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(campaign.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return false;

		array<SCR_CampaignBuildingCompositionComponent> compositions = {};
		int compositionCount = buildingManagerComponent.GetBuildingCompositions(m_MilitaryBase, compositions);
		foreach (SCR_CampaignBuildingCompositionComponent composition : compositions)
		{
			if (!composition.IsCompositionSpawned())
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode(this))
			return;

		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(campaign.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		buildingManagerComponent.GetOnAnyCompositionSpawned().Insert(OnAnyCompositionSpawned);
		buildingManagerComponent.GetOnCompositionUnregistered().Insert(OnCompositionUnregistered);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_BuildingCampaignMilitaryBaseTaskEntity()
	{
		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(campaign.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (buildingManagerComponent)
		{
			buildingManagerComponent.GetOnAnyCompositionSpawned().Remove(OnAnyCompositionSpawned);
			buildingManagerComponent.GetOnCompositionUnregistered().Remove(OnCompositionUnregistered);
		}
	}
}
