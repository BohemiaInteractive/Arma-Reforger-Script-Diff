[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingBudgetEditorComponentClass : SCR_BudgetEditorComponentClass
{
}

class SCR_CampaignBuildingBudgetEditorComponent : SCR_BudgetEditorComponent
{
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	protected SCR_ECharacterRank m_eHighestRank;
	
	protected static const int UNLIMITED_PROP_BUDGET = -1; 

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorActivateServer()
	{
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		array<ref SCR_RankID> ranks = factionManager.GetAllAvailableRanks();
		if (!ranks)
			return;

		ranks.Sort();

		SCR_RankID highestRankID = ranks[ranks.Count() - 1];
		if (!highestRankID)
			return;

		m_eHighestRank = highestRankID.GetRankID();
		
		RefreshBudgetSettings();
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorActivate()
	{
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));
		if (!m_CampaignBuildingComponent)
			return;

		RefreshBudgetSettings();
		m_CampaignBuildingComponent.GetOnProviderChanged().Insert(OnTargetBaseChanged);
		
		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (baseGameMode)
			baseGameMode.GetOnResourceTypeEnabledChanged().Insert(OnResourceTypeEnabledChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorDeactivate()
	{
		if (m_CampaignBuildingComponent)
			m_CampaignBuildingComponent.GetOnProviderChanged().Remove(OnTargetBaseChanged);
		
		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (baseGameMode)
			baseGameMode.GetOnResourceTypeEnabledChanged().Remove(OnResourceTypeEnabledChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnResourceTypeEnabledChanged(array<EResourceType> disabledResourceTypes)
	{
		RefreshResourcesComponent();
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("SCR_CampaignBuildingBudgetEditorComponent.RefreshResourcesComponent() should be used instead.")]
	protected bool RefreshSuppliesComponent()
	{
		if (!m_CampaignBuildingComponent)
			return false;

		SCR_CampaignSuppliesComponent providerSuppliesComponent;
		if (m_CampaignBuildingComponent.GetProviderSuppliesComponent(providerSuppliesComponent))
		{
			if (m_SuppliesComponent)
			{
				if (m_SuppliesComponent != providerSuppliesComponent)
				{
					m_SuppliesComponent.m_OnSuppliesChanged.Remove(OnBaseSuppliesChanged);
				}
			}
			else
			{
				providerSuppliesComponent.m_OnSuppliesChanged.Insert(OnBaseSuppliesChanged);
			}
		}

		m_SuppliesComponent = providerSuppliesComponent;

		return m_SuppliesComponent != null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseResourcesChanged(SCR_ResourceConsumer consumer, int previousValue)
	{
		RefreshResourcesComponent();
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("SCR_CampaignBuildingBudgetEditorComponent.OnBaseResourcesChanged() should be used instead.")]
	protected void OnBaseSuppliesChanged(int supplies)
	{
		RefreshResourcesComponent();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTargetBaseChanged(IEntity targetEntity)
	{
		RefreshBudgetSettings();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetMaxBudgetValue(EEditableEntityBudget type, out int maxBudget)
	{
		switch (type)
		{
			case EEditableEntityBudget.CAMPAIGN:
				if (!m_ResourceComponent)
					return true;
				
				SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				if (consumer)
					maxBudget = consumer.GetAggregatedMaxResourceValue();
				
				return true;

			case EEditableEntityBudget.RANK_PRIVATE:
				maxBudget = m_eHighestRank;
				return true;
			
			case EEditableEntityBudget.RANK_CORPORAL:
				maxBudget = m_eHighestRank;
				return true;

			case EEditableEntityBudget.RANK_SERGEANT:
				maxBudget = m_eHighestRank;
				return true;
			
			case EEditableEntityBudget.RANK_LIEUTENANT:
				maxBudget = m_eHighestRank;
				return true;
			
			case EEditableEntityBudget.RANK_CAPTAIN:
				maxBudget = m_eHighestRank;
				return true;
			
			case EEditableEntityBudget.RANK_MAJOR:
				maxBudget = m_eHighestRank;
				return true;
			
			case EEditableEntityBudget.RANK_COLONEL:
				maxBudget = m_eHighestRank;
				return true;
			
			case EEditableEntityBudget.RANK_GENERAL:
				maxBudget = m_eHighestRank;
				return true;

			case EEditableEntityBudget.ESTABLISH_BASE:
				maxBudget = CanEstablishBase();
				return true;

			case EEditableEntityBudget.PROPS:
				maxBudget = GetProviderMaxValue(type);
				return true;
			
			case EEditableEntityBudget.COOLDOWN:
				if (HasCooldownTime())
					maxBudget = 0;
				else
					maxBudget = 1;
			
				return true;
			
			case EEditableEntityBudget.AI:
					maxBudget = GetProviderMaxValue(type);
				return true;
			
			case EEditableEntityBudget.AI_SERVER:
				AIWorld aiWorld = GetGame().GetAIWorld();
				if (!aiWorld)
					return true;
						
				maxBudget = aiWorld.GetLimitOfActiveAIs();
				return true;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override int GetCurrentBudgetValue(EEditableEntityBudget type)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		if (!providerComponent)
			return -1;
		
		switch (type)
		{
			case EEditableEntityBudget.CAMPAIGN:
				if (!m_ResourceComponent || !m_ResourceComponent.IsResourceTypeEnabled())
					return -1;
				
				SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				if (!consumer)
					return -1;
				
				return consumer.GetAggregatedMaxResourceValue() - consumer.GetAggregatedResourceValue();

			case EEditableEntityBudget.RANK_PRIVATE:
				return m_eHighestRank - GetUserRank();
			
			case EEditableEntityBudget.RANK_CORPORAL:
				return m_eHighestRank - GetUserRank();

			case EEditableEntityBudget.RANK_SERGEANT:
				return m_eHighestRank - GetUserRank();
			
			case EEditableEntityBudget.RANK_LIEUTENANT:
				return m_eHighestRank - GetUserRank();
			
			case EEditableEntityBudget.RANK_CAPTAIN:
				return m_eHighestRank - GetUserRank();
			
			case EEditableEntityBudget.RANK_MAJOR:
				return m_eHighestRank - GetUserRank();
			
			case EEditableEntityBudget.RANK_COLONEL:
				return m_eHighestRank - GetUserRank();
			
			case EEditableEntityBudget.RANK_GENERAL:
				return m_eHighestRank - GetUserRank();

			case EEditableEntityBudget.ESTABLISH_BASE:
				return !CanEstablishBase();

			case EEditableEntityBudget.PROPS:
				return providerComponent.GetCurrentPropValue();
			
			case EEditableEntityBudget.COOLDOWN:
				return HasCooldownTime();
			
			case EEditableEntityBudget.AI:
				return providerComponent.GetCurrentAIValue();
			
			case EEditableEntityBudget.AI_SERVER:
				AIWorld aiWorld = GetGame().GetAIWorld();
				if (!aiWorld)
					return -1;
			
				return aiWorld.GetCurrentNumOfActiveAIs();
		}

		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return max prop value for a current provider. This number limits the number of prefabs (compositions) buildable with this provider.
	int GetProviderMaxValue(EEditableEntityBudget budget)
	{
		if (!m_CampaignBuildingComponent)
			return UNLIMITED_PROP_BUDGET;
		
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		if (!providerComponent)
			return UNLIMITED_PROP_BUDGET;
		
		return providerComponent.GetMaxBudgetValue(budget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return current cooldown timer state.
	bool HasCooldownTime()
	{		
		if (!m_CampaignBuildingComponent)
			return false;
		
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		if (!providerComponent)
			return false;
		
		if (!m_Manager)
			return false;
		
		return providerComponent.HasCooldownSet(m_Manager.GetPlayerID());
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCooldownTime()
	{
		if (!m_CampaignBuildingComponent)
			return 0;
		
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		if (!providerComponent)
			return 0;

		return providerComponent.GetCooldownValue(m_Manager.GetPlayerID());
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanEstablishBase()
	{
		// Establish base is allowed in others game modes like Gamemaster
		SCR_GameModeCampaign campaignGameMode = SCR_GameModeCampaign.GetInstance();
		if (!campaignGameMode)
			return true;

		if (!m_CampaignBuildingComponent)
			return false;

		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		if (!providerComponent)
			return false;

		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return false;

		Faction faction = SCR_FactionManager.SGetLocalPlayerFaction();
		if (!faction)
			return false;

		if (!campaignGameMode.GetBaseManager().CanFactionBuildNewBase(faction))
			return false;

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return false;

		array<SCR_Task> tasks = {};
		taskSystem.GetTasksByState(tasks, SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED, faction.GetFactionKey());

		SCR_EstablishBaseTaskEntity establishTask;
		SCR_EstablishBaseTaskEntity nearestRelevantTask;

		float distance;
		float distanceToNearestRelevantTask;

		foreach (SCR_Task task : tasks)
		{
			establishTask = SCR_EstablishBaseTaskEntity.Cast(task);

			if (!establishTask)
				continue;

			distance = vector.DistanceSqXZ(providerComponent.GetOwner().GetOrigin(), establishTask.GetOrigin());

			if (!nearestRelevantTask || distance < distanceToNearestRelevantTask)
			{
				nearestRelevantTask = establishTask;
				distanceToNearestRelevantTask = distance;
			}
		}

		int threshold = SCR_CampaignFactionCommanderHandlerComponent.Cast(SCR_FactionCommanderHandlerComponent.GetInstance()).GetBaseEstablishingRadius();

		if (!nearestRelevantTask || distanceToNearestRelevantTask > (threshold * threshold))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ECharacterRank GetUserRank()
	{
		int playerId = SCR_PlayerController.GetLocalPlayerId();

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return SCR_ECharacterRank.INVALID;
		
		return SCR_CharacterRankComponent.GetCharacterRank(playerController.GetControlledEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected override void OnEntityCoreBudgetUpdatedOwner(EEditableEntityBudget entityBudget, int budgetValue, int budgetChange, bool sendBudgetMaxEvent, bool budgetMaxReached)
	{
		int maxBudget;
		GetMaxBudgetValue(EEditableEntityBudget.PROPS, maxBudget);
		Event_OnBudgetUpdated.Invoke(EEditableEntityBudget.PROPS, GetCurrentBudgetValue(EEditableEntityBudget.PROPS), budgetValue, maxBudget);
		
		if (entityBudget != EEditableEntityBudget.CAMPAIGN)
			return;

		if (!m_ResourceComponent)
			return;
		
		SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		
		if (!consumer)
			return;
		
		if (consumer.GetAggregatedResourceValue() - budgetChange <= 0)
			Event_OnBudgetMaxReached.Invoke(entityBudget, true);
	}

	//------------------------------------------------------------------------------------------------
	protected bool RefreshResourcesComponent()
	{
		return m_CampaignBuildingComponent && m_CampaignBuildingComponent.GetProviderResourceComponent(m_ResourceComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void RefreshBudgetSettings()
	{
		if (!m_CampaignBuildingComponent)
			return;
		
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		if (!providerComponent)
			return;
		
		array<ref EEditableEntityBudget> budgetsType = {};
		providerComponent.GetBudgetTypesToEvaluate(budgetsType);

		SCR_EditableEntityCoreBudgetSetting budget;
		
		for (int i = budgetsType.Count() - 1; i >= 0; i--)
		{
			EEditableEntityBudget budgetType = budgetsType[i];
			
			int maxBudgetValue;
			
			if (m_EntityCore.GetBudget(budgetType, budget) && GetMaxBudgetValue(budgetType, maxBudgetValue))
				m_BudgetSettingsMap.Insert(budgetType, budget);
		}

		RefreshResourcesComponent();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanPlaceEntitySource(IEntityComponentSource editableEntitySource, out EEditableEntityBudget blockingBudget, bool isPlacingPlayer = false, bool updatePreview = true, bool showNotification = true)
	{
		bool canPlace = true;
		if (IsBudgetCapEnabled() && editableEntitySource)
		{
			array<ref SCR_EntityBudgetValue> budgetCosts = {};
			if (!GetEntitySourcePreviewBudgetCosts(editableEntitySource, budgetCosts))
				canPlace = false;

			// Clear budget cost when placing as player
			if (isPlacingPlayer)
				budgetCosts.Clear();

			if (updatePreview)
				UpdatePreviewCost(budgetCosts);

			canPlace = canPlace && CanPlace(budgetCosts, blockingBudget);
		}
		return CanPlaceResultCampaignBuilding(canPlace, showNotification, blockingBudget);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanPlaceResultCampaignBuilding(bool canPlace, bool showNotification, EEditableEntityBudget blockingBudget)
	{
		if (showNotification)
		{
			Rpc(CanPlaceOwnerCampaignBuilding, canPlace, blockingBudget);
		}
		return canPlace;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] canPlace
	//! \param[in] blockingBudget
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void CanPlaceOwnerCampaignBuilding(bool canPlace, EEditableEntityBudget blockingBudget)
	{
		if (!canPlace)
		{
			switch (blockingBudget)
			{
				case EEditableEntityBudget.CAMPAIGN:
				{
					GetManager().SendNotification(ENotification.EDITOR_PLACING_NO_ENOUGH_SUPPLIES);
					break;
				}

				case EEditableEntityBudget.RANK_PRIVATE:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_TOO_LOW, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.PRIVATE);
					break;
				}
				
				case EEditableEntityBudget.RANK_CORPORAL:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_TOO_LOW, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.CORPORAL);
					break;
				}

				case EEditableEntityBudget.RANK_SERGEANT:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_TOO_LOW, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.SERGEANT);
					break;
				}
				
				case EEditableEntityBudget.RANK_LIEUTENANT:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_TOO_LOW, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.LIEUTENANT);
					break;
				}
				
				case EEditableEntityBudget.RANK_CAPTAIN:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_TOO_LOW, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.CAPTAIN);
					break;
				}
				
				case EEditableEntityBudget.RANK_MAJOR:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_TOO_LOW, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.MAJOR);
					break;
				}
				
				case EEditableEntityBudget.RANK_COLONEL:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_TOO_LOW, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.COLONEL);
					break;
				}
				
				case EEditableEntityBudget.RANK_GENERAL:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_TOO_LOW, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.GENERAL);
					break;
				}

				case EEditableEntityBudget.ESTABLISH_BASE:
				{
					SCR_GameModeCampaign campaignGameMode = SCR_GameModeCampaign.GetInstance();
					Faction faction = SCR_FactionManager.SGetLocalPlayerFaction();
					if (campaignGameMode && faction && !campaignGameMode.GetBaseManager().CanFactionBuildNewBase(faction))
						GetManager().SendNotification(ENotification.EDITOR_PLACING_ESTABLISH_BASE_LIMIT_REACHED);
					else
						GetManager().SendNotification(ENotification.EDITOR_PLACING_BUDGET_ESTABLISH_BASE);
					break;
				}

				case EEditableEntityBudget.PROPS:
				{
					GetManager().SendNotification(ENotification.EDITOR_PLACING_NO_MORE_COMPOSITIONS_AT_BASE);
					break;
				}
				
				case EEditableEntityBudget.COOLDOWN:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_COOLDOWN, GetCooldownTime());
					break;
				}
				
				case EEditableEntityBudget.AI:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_AILIMIT);
					break;
				}
				
				case EEditableEntityBudget.AI_SERVER:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_BUDGET_MAX);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool GetMaxBudget(EEditableEntityBudget type, out SCR_EntityBudgetValue budget)
	{
		const bool valid = super.GetMaxBudget(type, budget);
		
		if(!valid)
			return false;

		//if some checks fail, the budget will not change
		int newMaxBudget = budget.GetBudgetValue();

		switch (type)
		{
			case EEditableEntityBudget.CAMPAIGN:
			{
				if (!m_ResourceComponent)
					break;
				
				SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				if (!consumer)
					break;
				
				newMaxBudget = consumer.GetAggregatedResourceValue();
				break;
			}
			
			case EEditableEntityBudget.RANK_PRIVATE:
			{
				newMaxBudget = GetUserRank();
				break;
			}

			case EEditableEntityBudget.RANK_CORPORAL:
			{
				newMaxBudget = GetUserRank();
				break;
			}

			case EEditableEntityBudget.RANK_SERGEANT:
			{
				newMaxBudget = GetUserRank();
				break;
			}
			
			case EEditableEntityBudget.RANK_LIEUTENANT:
			{
				newMaxBudget = GetUserRank();
				break;
			}
			
			case EEditableEntityBudget.RANK_CAPTAIN:
			{
				newMaxBudget = GetUserRank();
				break;
			}
			
			case EEditableEntityBudget.RANK_MAJOR:
			{
				newMaxBudget = GetUserRank();
				break;
			}
			
			case EEditableEntityBudget.RANK_COLONEL:
			{
				newMaxBudget = GetUserRank();
				break;
			}
			
			case EEditableEntityBudget.RANK_GENERAL:
			{
				newMaxBudget = GetUserRank();
				break;
			}

			case EEditableEntityBudget.ESTABLISH_BASE:
			{
				newMaxBudget = !CanEstablishBase();
				break;
			}

			case EEditableEntityBudget.PROPS:
			{					
				newMaxBudget = GetProviderMaxValue(EEditableEntityBudget.PROPS);
				break;
			}
			
			case EEditableEntityBudget.COOLDOWN:
			{					
				newMaxBudget = HasCooldownTime();
				break;
			}
			
			case EEditableEntityBudget.AI:
			{					
				newMaxBudget = GetProviderMaxValue(EEditableEntityBudget.AI);
				break;
			}
			
			case EEditableEntityBudget.AI_SERVER:
			{	
				AIWorld aiWorld = GetGame().GetAIWorld();
				if (!aiWorld)
					break;
						
				newMaxBudget = aiWorld.GetLimitOfActiveAIs();
				break;
			}
		}

		budget.SetBudgetValue(newMaxBudget);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanPlace(notnull array<ref SCR_EntityBudgetValue> budgetCosts, out EEditableEntityBudget blockingBudget)
	{
		if (!IsBudgetCapEnabled() || budgetCosts.IsEmpty()) 
			return true;
		
		array<EEditableEntityBudget> blockingBudgets = {};
		int initialPriorityOrder = -1;
		EEditableEntityBudget blockingBudgetCandidate = -1;
		
		foreach (SCR_EntityBudgetValue budgetCost : budgetCosts)
		{
			if (!CanPlace(budgetCost, blockingBudget))
			{
				if (IsBudgetCapEnabled(blockingBudget))
				{
					SCR_EditableEntityCoreBudgetSetting budgetSettings = GetBudgetSetting(blockingBudget);
					if (!budgetSettings)
						continue;
				
					// promote budget to candidate if it's non priority budget, only when there wasn't set any priority budget yet.
					SCR_BudgetUIInfo budgetUIInfo = SCR_BudgetUIInfo.Cast(budgetSettings.GetInfo());
					if (!budgetUIInfo && initialPriorityOrder == -1)
					{
						blockingBudgetCandidate = blockingBudget;
						continue;
					}
					
					if (!budgetUIInfo)
						continue;

					// this budget has higher priority then any other before, promote it as a priority budget.
					if (initialPriorityOrder < budgetUIInfo.GetPriorityOrder())
					{
						blockingBudgetCandidate = blockingBudget;
						initialPriorityOrder = budgetUIInfo.GetPriorityOrder();
					}
				}
			};
		}
		
		if (blockingBudgetCandidate == -1)
			return true;
		
		blockingBudget = blockingBudgetCandidate;

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the given budget is on the list of budgets used by a provider.
	//! \param[in] blockingBudget
	//! \return
	bool IsBudgetCapEnabled(EEditableEntityBudget blockingBudget)
	{
		if (!m_CampaignBuildingComponent)
			return true;
		
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		if (!providerComponent)
			return true;
		
		return providerComponent.IsBudgetToEvaluate(blockingBudget);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetEntityPreviewBudgetCosts(SCR_EditableEntityUIInfo entityUIInfo, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		if (!entityUIInfo)
			return false;
		
		SCR_EditableGroupUIInfo groupUIInfo = SCR_EditableGroupUIInfo.Cast(entityUIInfo);
		if (groupUIInfo)
		{
			entityUIInfo.GetEntityAndChildrenBudgetCost(budgetCosts);
				
			if (!entityUIInfo.GetEntityBudgetCost(budgetCosts))
				GetEntityTypeBudgetCost(entityUIInfo.GetEntityType(), budgetCosts);
		}
		else
		{
			array<ref SCR_EntityBudgetValue> entityChildrenBudgetCosts = {};
			
			if (!entityUIInfo.GetEntityBudgetCost(budgetCosts))
				GetEntityTypeBudgetCost(entityUIInfo.GetEntityType(), budgetCosts);
			
			entityUIInfo.GetEntityChildrenBudgetCost(entityChildrenBudgetCosts);
	
			SCR_EntityBudgetValue.MergeBudgetCosts(budgetCosts, entityChildrenBudgetCosts);
		}

		FilterAvailableBudgets(budgetCosts);
		
		return true;
	}
}
