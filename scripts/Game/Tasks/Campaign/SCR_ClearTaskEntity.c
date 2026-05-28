class SCR_ClearTaskEntityClass : SCR_TaskClass
{
}

class SCR_ClearTaskEntity : SCR_Task
{
	protected const int PERIODICAL_CHECK_INTERVAL = 5000; // ms

	[Attribute("50", UIWidgets.EditBox, "Area radius [m]", "0 inf")]
	protected float m_fAreaRadius;

	protected ref array<IEntity> m_aObstacleEntities = {};
	protected ref array<int> m_aAssigneesInArea = {}; //! array of playerIDs

	protected RplComponent m_RplComponent;
	protected bool m_bWasActivated;

	//------------------------------------------------------------------------------------------------
	protected void PeriodicalCheck()
	{
		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs || assigneePlayerIDs.IsEmpty())
			return;

		SCR_ChimeraCharacter character;
		CharacterControllerComponent charControl;
		PlayerManager playerManager = GetGame().GetPlayerManager();
		vector taskPosition = GetTaskPosition();
		m_aAssigneesInArea.Clear();

		foreach (int playerId : assigneePlayerIDs)
		{
			character = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerId));
			if (!character)
				continue;

			charControl = character.GetCharacterController();
			if (!charControl || charControl.IsDead())
				continue;

			if (vector.DistanceSqXZ(character.GetOrigin(), taskPosition) > m_fAreaRadius * m_fAreaRadius)
				continue;

			m_aAssigneesInArea.Insert(playerId);
		}

		if (m_aAssigneesInArea.IsEmpty())
			return;

		int obstacleCount = m_aObstacleEntities.Count();
		m_aObstacleEntities.Clear();
		GetGame().GetWorld().QueryEntitiesBySphere(taskPosition, m_fAreaRadius, QueryCallback);

		if (m_bWasActivated && obstacleCount > m_aObstacleEntities.Count())
		{
			// Obstacle was removed, objective successful
			SetTaskState(SCR_ETaskState.COMPLETED);
			m_TaskSystem.DeleteTask(this);
			return;
		}
		
		if (!m_bWasActivated)
		{
			m_bWasActivated = true;

			// Fail objective if nothing to be cleared found on first activation
			if (m_aObstacleEntities.IsEmpty())
			{
				SetTaskState(SCR_ETaskState.FAILED);
				m_TaskSystem.DeleteTask(this);
				return;
			}

			SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_CampaignBuildingManagerComponent));
			if (buildingManagerComponent)
				buildingManagerComponent.GetOnCompositionUnregistered().Insert(OnCompositionUnregistered);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Inserts compositions and active mines to list of obstacles
	protected bool QueryCallback(IEntity e)
	{
		SCR_CampaignBuildingCompositionComponent buildingComp = SCR_CampaignBuildingCompositionComponent.Cast(e.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (buildingComp && buildingComp.IsCompositionSpawned())
			m_aObstacleEntities.Insert(e);

		SCR_BaseTriggerComponent triggerComp = SCR_BaseTriggerComponent.Cast(e.FindComponent(SCR_BaseTriggerComponent));
		if (triggerComp && triggerComp.IsActivated())
			m_aObstacleEntities.Insert(e);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompositionUnregistered(SCR_CampaignBuildingCompositionComponent composition)
	{
		if (m_aObstacleEntities.IsEmpty())
			return;

		IEntity owner = composition.GetOwner();
		if (!owner || !m_aObstacleEntities.Contains(owner))
			return;

		// Obstacle cleared, objective successful
		SetTaskState(SCR_ETaskState.COMPLETED);
		m_TaskSystem.DeleteTask(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddXPReward()
	{
		if (m_RplComponent.IsProxy())
			return;

		SCR_XPHandlerComponent xpHandler = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		if (!xpHandler)
			return;

		foreach (int playerId : m_aAssigneesInArea)
		{
			xpHandler.AwardXP(playerId, SCR_EXPRewards.CLEAR_TASK_COMPLETED);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void SetTaskState(SCR_ETaskState state)
	{
		if (state == SCR_ETaskState.COMPLETED)
		{
			AddXPReward();
			GetGame().GetCallqueue().Remove(PeriodicalCheck);
		}
		else if (state == SCR_ETaskState.FAILED || state == SCR_ETaskState.CANCELLED)
		{
			GetGame().GetCallqueue().Remove(PeriodicalCheck);
		}

		super.SetTaskState(state);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode(this))
			return;

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		GetGame().GetCallqueue().CallLater(PeriodicalCheck, PERIODICAL_CHECK_INTERVAL, true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_ClearTaskEntity()
	{
		if (SCR_Global.IsEditMode(this))
			return;

		GetGame().GetCallqueue().Remove(PeriodicalCheck);

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (buildingManagerComponent)
			buildingManagerComponent.GetOnCompositionUnregistered().Remove(OnCompositionUnregistered);
	}
}
