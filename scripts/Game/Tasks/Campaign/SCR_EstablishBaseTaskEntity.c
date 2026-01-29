class SCR_EstablishBaseTaskEntityClass : SCR_TaskClass
{
}

class SCR_EstablishBaseTaskEntity : SCR_Task
{
	protected RplComponent m_RplComponent;
	protected bool m_bPlayerInArea;
	protected bool m_bIsRunningCheckEstablishmentArea;
	protected int m_iBaseEstablishingRadiusSq;
	protected int m_iBaseEstablishingRadius;
	protected int m_iBaseMinDistanceSq;
	protected float m_fProviderBuildingRadius;
	protected SCR_CampaignBuildingManagerComponent m_BuildingManagerComponent;
	protected bool m_bHasHQSameFactionAsTask;
	protected SCR_CampaignFaction m_BuilderFaction;
	protected ref map<SCR_CampaignBuildingCompositionComponent, float> m_mPlacedHQs = new map<SCR_CampaignBuildingCompositionComponent, float>();

	protected const int DELETE_TASK_DELAY = 600;

	protected static ref ScriptInvokerVoid s_OnPlayerEnteredEstablishingArea;

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerVoid GetOnPlayerEnteredEstablishingArea()
	{
		if (!s_OnPlayerEnteredEstablishingArea)
			s_OnPlayerEnteredEstablishingArea = new ScriptInvokerVoid();

		return s_OnPlayerEnteredEstablishingArea;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddXPReward()
	{
		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		if (!comp || !m_RplComponent || m_RplComponent.IsProxy())
			return;

		// add XP to all players in the assigned group
		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs)
			return;

		foreach (int playerID : assigneePlayerIDs)
		{
			comp.AwardXP(playerID, SCR_EXPRewards.ESTABLISH_BASE_COMPLETED, 1.0, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckEstablishmentArea()
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;

		float distanceSq = vector.DistanceSqXZ(player.GetOrigin(), GetOrigin());

		if (!m_bPlayerInArea && distanceSq <= m_iBaseEstablishingRadiusSq)
		{
			m_bPlayerInArea = true;
			SCR_NotificationsComponent.SendLocal(ENotification.GROUP_TASK_ESTABLISH_BASE_ENABLED);

			if (s_OnPlayerEnteredEstablishingArea)
				s_OnPlayerEnteredEstablishingArea.Invoke();
		}

		if (m_bPlayerInArea && distanceSq > m_iBaseEstablishingRadiusSq)
		{
			m_bPlayerInArea = false;
			SCR_NotificationsComponent.SendLocal(ENotification.GROUP_TASK_ESTABLISH_BASE_DISABLED);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void StartCheckingEstablishmentArea()
	{
		if (m_bIsRunningCheckEstablishmentArea)
			return;

		if (!IsTaskAssignedTo(SCR_TaskExecutorPlayer.FromPlayerID(SCR_PlayerController.GetLocalPlayerId())))
			return;

		m_bIsRunningCheckEstablishmentArea = true;
		GetGame().GetCallqueue().CallLater(CheckEstablishmentArea, SCR_GameModeCampaign.DEFAULT_DELAY, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void StopCheckingEstablishmentArea()
	{
		if (IsTaskAssignedTo(SCR_TaskExecutorPlayer.FromPlayerID(SCR_PlayerController.GetLocalPlayerId())))
			return;

		m_bPlayerInArea = false;
		m_bIsRunningCheckEstablishmentArea = false;
		GetGame().GetCallqueue().Remove(CheckEstablishmentArea);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskAssigneeAdded(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (task != this)
			return;

		StartCheckingEstablishmentArea();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskAssigneeRemoved(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (task != this)
			return;

		StopCheckingEstablishmentArea();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_MilitaryBaseComponent GetBase(vector position, bool isAllowedCampaignBase = false)
	{
		SCR_MilitaryBaseSystem baseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (!baseSystem)
			return null;

		array<SCR_MilitaryBaseComponent> bases = {};
		baseSystem.GetBases(bases);

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			if (vector.DistanceSqXZ(base.GetOwner().GetOrigin(), position) <= (base.GetRadius() * base.GetRadius()))
			{
				if (!isAllowedCampaignBase && SCR_CampaignMilitaryBaseComponent.Cast(base))
					continue;

				return base;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseBuilt(notnull SCR_CampaignMilitaryBaseComponent campaignBase, Faction faction)
	{
		float distanceSq = vector.DistanceSqXZ(campaignBase.GetOwner().GetOrigin(), GetOrigin());

		if (distanceSq <= Math.Pow(m_iBaseEstablishingRadius + m_fProviderBuildingRadius, 2))
		{
			if (m_bHasHQSameFactionAsTask)
			{
				m_TaskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED); // HQ was built and the hologram was placed by a builder with the same faction.

				// change faction of the HQ composition if not match with builder's faction
				if (campaignBase.GetCampaignFaction() != m_BuilderFaction)
				{
					campaignBase.SetFaction(m_BuilderFaction);

					SCR_MilitaryBaseComponent base = GetBase(campaignBase.GetOwner().GetOrigin());
					if (base)
						base.SetFaction(m_BuilderFaction);
				}
			}
			else
			{
				m_TaskSystem.SetTaskState(this, SCR_ETaskState.FAILED);
			}

			DeleteTask();
		}
		else if (distanceSq <= m_iBaseMinDistanceSq)
		{
			if (m_bHasHQSameFactionAsTask)
				m_TaskSystem.SetTaskState(this, SCR_ETaskState.CANCELLED);
			else
				m_TaskSystem.SetTaskState(this, SCR_ETaskState.FAILED);

			DeleteTask();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskStateChanged(SCR_Task task, SCR_ETaskState newState)
	{
		if (task != this)
			return;

		if (newState == SCR_ETaskState.COMPLETED)
		{
			AddXPReward();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHQCompositionSpawned(bool spawned)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		m_bHasHQSameFactionAsTask = false;

		foreach (SCR_CampaignBuildingCompositionComponent buildingComposition, float buildingRadius : m_mPlacedHQs)
		{
			if (!buildingComposition || !buildingComposition.IsCompositionSpawned())
				continue;

			SCR_CampaignFaction builderFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetPlayerFaction(buildingComposition.GetBuilderId()));
			if (!builderFaction)
				continue;

			SCR_CampaignFaction campaignFaction;
			foreach (string factionKey : m_TaskData.m_aOwnerFactionKeys)
			{
				if (factionKey.IsEmpty())
					continue;

				campaignFaction = SCR_CampaignFaction.Cast(factionManager.GetFactionByKey(factionKey));
				if (!campaignFaction)
					continue;

				if (campaignFaction == builderFaction)
				{
					m_bHasHQSameFactionAsTask = true;
					m_BuilderFaction = builderFaction;
					m_fProviderBuildingRadius = buildingRadius;
					return;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntitySpawnedByProvider(int prefabID, SCR_EditableEntityComponent editableEntity, int playerId, SCR_CampaignBuildingProviderComponent provider)
	{
		if (!editableEntity || !provider)
			return;

		SCR_CampaignBuildingCompositionComponent buildingCompositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!buildingCompositionComponent)
			return;

		if (!IsHQService(editableEntity))
			return;

		vector entityPos;
		editableEntity.GetPos(entityPos);

		float providerBuildingRadius = provider.GetBuildingRadius();
		if (vector.DistanceSqXZ(entityPos, GetOrigin()) > m_iBaseMinDistanceSq)
			return;

		m_mPlacedHQs.Set(buildingCompositionComponent, providerBuildingRadius);

		buildingCompositionComponent.GetOnCompositionSpawned().Insert(OnHQCompositionSpawned);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsHQService(SCR_EditableEntityComponent editableEntity)
	{
		SCR_EditableEntityUIInfo prefabInfo = SCR_EditableEntityUIInfo.Cast(editableEntity.GetInfo());
		if (!prefabInfo)
			return false;

		array<EEditableEntityLabel> entityLabels = {};
		prefabInfo.GetEntityLabels(entityLabels);
		return entityLabels.Contains(EEditableEntityLabel.SERVICE_HQ);
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteTask()
	{
		if (!m_TaskSystem)
			return;

		m_TaskSystem.DeleteTask(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDataLoaded()
	{
		StartCheckingEstablishmentArea();
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		bool loaded = super.RplLoad(reader);
		if (loaded)
			OnDataLoaded();

		return loaded;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode(this))
			return;

		SCR_CampaignFactionCommanderHandlerComponent handlerComponent	= SCR_CampaignFactionCommanderHandlerComponent.Cast(SCR_FactionCommanderHandlerComponent.GetInstance());
		if (!handlerComponent)
			return;

		m_iBaseEstablishingRadius = handlerComponent.GetBaseEstablishingRadius();
		m_iBaseEstablishingRadiusSq = m_iBaseEstablishingRadius * m_iBaseEstablishingRadius;
		int baseMinDistance = SCR_CampaignFactionCommanderHandlerComponent.Cast(SCR_FactionCommanderHandlerComponent.GetInstance()).GetBaseMinDistance(SCR_ECampaignBaseType.BASE);
		m_iBaseMinDistanceSq = baseMinDistance * baseMinDistance;

		// run only on client with UI
		if (!System.IsConsoleApp())
		{
			GetOnTaskAssigneeAdded().Insert(OnTaskAssigneeAdded);
			GetOnTaskAssigneeRemoved().Insert(OnTaskAssigneeRemoved);
		}

		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		GetOnTaskStateChanged().Insert(OnTaskStateChanged);

		SCR_GameModeCampaign gameMode = SCR_GameModeCampaign.GetInstance();
		if (!gameMode)
			return;

		SCR_CampaignMilitaryBaseManager baseManager = gameMode.GetBaseManager();
		if (baseManager)
			baseManager.GetOnBaseBuilt().Insert(OnBaseBuilt);

		m_BuildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!m_BuildingManagerComponent)
			return;

		m_BuildingManagerComponent.GetOnEntitySpawnedByProvider().Insert(OnEntitySpawnedByProvider);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_EstablishBaseTaskEntity()
	{
		GetOnTaskAssigneeAdded().Remove(OnTaskAssigneeAdded);
		GetOnTaskAssigneeRemoved().Remove(OnTaskAssigneeRemoved);

		GetOnTaskStateChanged().Remove(OnTaskStateChanged);

		SCR_GameModeCampaign gameMode = SCR_GameModeCampaign.GetInstance();
		if (!gameMode)
			return;

		SCR_CampaignMilitaryBaseManager baseManager = gameMode.GetBaseManager();
		if (baseManager)
			baseManager.GetOnBaseBuilt().Remove(OnBaseBuilt);

		if (!m_BuildingManagerComponent)
			return;

		m_BuildingManagerComponent.GetOnEntitySpawnedByProvider().Remove(OnEntitySpawnedByProvider);

		foreach (SCR_CampaignBuildingCompositionComponent buildingComposition, float buildingRadius : m_mPlacedHQs)
		{
			if (!buildingComposition)
				continue;

			buildingComposition.GetOnCompositionSpawned().Remove(OnHQCompositionSpawned);
		}
	}
}
