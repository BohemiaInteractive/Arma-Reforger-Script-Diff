[ComponentEditorProps(category: "GameScripted/Editor", description: "Main conflict editor component to handle building mode placing", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingPlacingEditorComponentClass : SCR_PlacingEditorComponentClass
{
	[Attribute(params: "et", desc: "List of prefabs that are ignored for clipping check.")]
	protected ref array<ResourceName> m_aClippingCheckIgnoredPrefabs;

	bool ContainPrefab(ResourceName res)
	{
		if (m_aClippingCheckIgnoredPrefabs.Contains(res))
			return true;

		return false;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingPlacingEditorComponent : SCR_PlacingEditorComponent
{
	[Attribute("{C7EE4C198B641E21}Sounds/Editor/BaseBuilding/Editor_BaseBuilding.acp", UIWidgets.ResourceNamePicker, "Sound project file to be used for building", "acp")]
	protected ResourceName m_sSoundFile;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "WP that is spawned as default for placed AI.", "et")]
	protected ResourceName m_sDefaultAIWP;

	protected bool m_bCanBeCreated = true;
	protected SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	protected ScriptedGameTriggerEntity m_AreaTrigger;
	protected IEntity m_Provider;
	protected ECantBuildNotificationType m_eBlockingReason;
	protected SCR_EditablePreviewEntity m_PreviewEnt;

	
	//different because this cant be a global map, but one map per base, probably gotta put this somewhere else lol
	ref map<EEditableEntityBudget, int> m_accumulatedCampaignBudgetChanges = new map<EEditableEntityBudget, int>;
	//------------------------------------------------------------------------------------------------
	//! Return the base which belongs to a provider (if any)
	protected bool GetProviderBase(out SCR_MilitaryBaseComponent base)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(m_Provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return false;

		base = SCR_MilitaryBaseComponent.Cast(m_Provider.FindComponent(SCR_MilitaryBaseComponent));
		if (!base)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the preview of prefab to build is created.
	protected void OnPreviewCreated(SCR_EditablePreviewEntity previewEnt)
	{
		if (!previewEnt)
			return;

		SCR_CampaignBuildingPlacingObstructionEditorComponent obstructionComponent = SCR_CampaignBuildingPlacingObstructionEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingPlacingObstructionEditorComponent, true, true));
		if (!obstructionComponent)
			return;

		obstructionComponent.OnPreviewCreated(previewEnt);
	}

	//------------------------------------------------------------------------------------------------
	//! Set the initial state of the preview can / can't be created (based on the place where player place the preview into the world.)
	protected void SetInitialCanBeCreatedState(notnull SCR_EditablePreviewEntity previewEnt)
	{
		if (!previewEnt)
			return;

		float distance = vector.DistanceSq(m_AreaTrigger.GetOrigin(), previewEnt.GetOrigin());
		if (distance < m_AreaTrigger.GetSphereRadius() * m_AreaTrigger.GetSphereRadius())
			m_bCanBeCreated = true;

		m_bCanBeCreated = false;
		m_eBlockingReason = ECantBuildNotificationType.OUT_OF_AREA;
		return;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBeforeEntityCreatedServer(ResourceName prefab)
	{
		// Get SCR_CampaignBuildingManagerComponent (on Game mode) here and set temporary owner.
		BaseGameMode gameMode = GetGame().GetGameMode();

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent || !m_CampaignBuildingComponent)
			return;

		buildingManagerComponent.SetTemporaryProvider(m_CampaignBuildingComponent.GetProviderEntity());
	}

	//-----------------------------------------------------------------------------------------------
	override bool IsThereEnoughBudgetToSpawn(IEntityComponentSource entitySource)
	{	
		if(!entitySource)
			return false;
		
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(m_Provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return true;
		
		///-------------------------------
		array<ref SCR_EntityBudgetValue> budgetCosts = {};
		map<EEditableEntityBudget, int> tempBudgetAggregation = m_accumulatedBudgetChanges;
		SCR_EditableEntityCoreBudgetSetting budgetSettings;
		
		m_BudgetManager.GetBudgetCostsDontDiscardCampaignBudget(entitySource, budgetCosts);	

		return providerComponent.IsThereEnoughBudgetToSpawn(budgetCosts);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnEntityCreatedServer(array<SCR_EditableEntityComponent> entities)
	{
		if (!m_CampaignBuildingComponent)
			return;

		int cooldownTime, aiBudgetValue;
		int count = entities.Count();
		SCR_EditorLinkComponent linkComponent;
		SCR_EditableGroupComponent editableGroupComponent;
		SCR_CampaignBuildingCompositionComponent compositionComponent;
		SCR_AIGroup aiGroup;
		
		for (int i = 0; i < count; i++)
		{
			IEntity entityOwner = entities[i].GetOwnerScripted();
			if (!entityOwner)
				continue;
			
			linkComponent = SCR_EditorLinkComponent.Cast(entityOwner.FindComponent(SCR_EditorLinkComponent));
			if (!linkComponent)
				SCR_EditorLinkComponent.IgnoreSpawning(false);
			
			editableGroupComponent = SCR_EditableGroupComponent.Cast(entities[i]);
			if (editableGroupComponent)
			{
				// Set Free Roam building Ai flag both group and characters in it to be able to process them on saving / loading as they use it's own struct.
				SetAiFlag(editableGroupComponent);
				
				aiGroup = editableGroupComponent.GetAIGroupComponent();
				if (aiGroup)
				{
					aiGroup.GetOnInit().Insert(InitGroup);
				
					EntitySpawnParams params = EntitySpawnParams();
					params.TransformMode = ETransformMode.WORLD;
					params.Transform[3] = aiGroup.GetOrigin();
					SCR_AIWaypoint defendWP = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(m_sDefaultAIWP), null, params));

					aiGroup.AddWaypointAt(defendWP, 0);
				}
			}

			compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(entityOwner.FindComponent(SCR_CampaignBuildingCompositionComponent));
			if (compositionComponent)
				SetProviderAndBuilder(compositionComponent);

			if (!UseCooldown())
				continue;
			
			cooldownTime = GetEntityBudgetValue(entities[i], EEditableEntityBudget.COOLDOWN);
			aiBudgetValue = GetEntityBudgetValue(entities[i], EEditableEntityBudget.AI);
		}
		
		SetCooldownTimer(cooldownTime);
		SetAIBudget(aiBudgetValue);
	}
		
	//------------------------------------------------------------------------------------------------
	//! The placement is driven by a cooldown set on server. However for a client needs (to show him a remaining time) the cooldown has to be set as well. The difference in time between server and client is acceptable.
	protected void GetCooldownTimeClient(int prefabID, SCR_EditableEntityComponent entity)
	{
		SetCooldownTimer(GetEntityBudgetValue(entity, EEditableEntityBudget.COOLDOWN));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetAIBudgetClient(int prefabID, SCR_EditableEntityComponent entity)
	{
		SetAIBudget(GetEntityBudgetValue(entity, EEditableEntityBudget.AI));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get value of given budget for given entity.
	int GetEntityBudgetValue(notnull SCR_EditableEntityComponent entity, EEditableEntityBudget budget)
	{
		array<ref SCR_EntityBudgetValue> outBudgets = {};
		entity.GetEntityChildrenBudgetCost(outBudgets);
		
		if (outBudgets.IsEmpty())
			entity.GetEntityBudgetCost(outBudgets);
		
		foreach (SCR_EntityBudgetValue outBudget: outBudgets)
		{
			if (outBudget.GetBudgetType() == budget)
				return outBudget.GetBudgetValue();
		}
		
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Set the provider and builder entity to composition. Provider is the entity to which a composition belong to. For example a base, supply truck... Builder is a player who build it.
	protected void SetProviderAndBuilder(notnull SCR_CampaignBuildingCompositionComponent compositionComponent)
	{
		IEntity provider = m_CampaignBuildingComponent.GetProviderEntity();
		if (!provider)
			return;

		SCR_EditorManagerEntity managerEnt = GetManager();
		if (!managerEnt)
			return;

		int id = managerEnt.GetPlayerID();

		compositionComponent.SetBuilderId(id);
		compositionComponent.SetProviderEntity(provider);

		// Don't set up this hook if the provider is a base. We don't want to change the ownership here
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;

		SCR_MilitaryBaseComponent base = providerComponent.GetMilitaryBaseComponent();
		if (base)
			return;

		SCR_EditorModeEntity ent = SCR_EditorModeEntity.Cast(GetOwner());
		compositionComponent.SetClearProviderEvent(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init spawned AI group (set events and flags)
	void InitGroup(SCR_AIGroup aiGroup)
	{
		if (!aiGroup)
			return;
		
		aiGroup.GetOnInit().Remove(InitGroup);
		
		array<AIAgent> outAgents = {};
		IEntity ent;
		SCR_EditableEntityComponent editableEntityComponent;
		SCR_CampaignBuildingProviderComponent providerComponent;
		
		aiGroup.GetAgents(outAgents);
		
		foreach (AIAgent agent: outAgents)
		{
			ent = agent.GetControlledEntity();
			editableEntityComponent = SCR_EditableEntityComponent.Cast(ent.FindComponent(SCR_EditableEntityComponent));
			if (editableEntityComponent)
				SetAiFlag(editableEntityComponent);
			
			providerComponent = SCR_CampaignBuildingProviderComponent.Cast(m_Provider.FindComponent(SCR_CampaignBuildingProviderComponent));
			if (!providerComponent)
				continue;
			
			providerComponent.SetOnEntityKilled(agent.GetControlledEntity());
		}
	}
		
	//------------------------------------------------------------------------------------------------
	void SetAiFlag(SCR_EditableEntityComponent component)
	{
		component.SetEntityFlag(EEditableEntityFlag.FREE_ROAM_BUILDING_AI, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlaceEntityServer(int prefabID, SCR_EditableEntityComponent entity)
	{
		vector position = entity.GetOwner().GetOrigin();
		Rpc(PlaySoundEvent, position, m_sSoundFile);
		PlaySoundEvent(position, m_sSoundFile);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void PlaySoundEvent(vector pos, string soundEvent)
	{
		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		transform[3] = pos;

		if (soundEvent && pos)
			AudioSystem.PlayEvent(soundEvent, SCR_SoundEvent.SOUND_BUILD, transform, new array<string>, new array<float>);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override protected void CreateEntityServer(SCR_EditorPreviewParams params, RplId prefabID, int playerID, int entityIndex, bool isQueue, array<RplId> recipientIds, bool canBePlayer, RplId holderId)
	{
		// Cancel spawning of the composition instantly and spawn a composition layout instead.
		SCR_EditorLinkComponent.IgnoreSpawning(true);

		// Cancel a services registration to base - register once the final structure is erected.
		SCR_ServicePointComponent.SpawnAsOffline(true);
		
		super.CreateEntityServer(params, prefabID, playerID, entityIndex, isQueue, recipientIds, canBePlayer, holderId);
	}

	//------------------------------------------------------------------------------------------------
	//! Search for the outline that is assigned to this composition to be spawned.
	ResourceName GetOutlineToSpawn(notnull SCR_EditableEntityComponent entity)
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return string.Empty;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return string.Empty;

		SCR_CampaignBuildingCompositionOutlineManager outlineManager = buildingManagerComponent.GetOutlineManager();
		if (!outlineManager)
			return string.Empty;

		return outlineManager.GetCompositionOutline(entity);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanCreateEntity(out ENotification outNotification = -1, inout SCR_EPreviewState previewStateToShow = SCR_EPreviewState.PLACEABLE)
	{
		SCR_CampaignBuildingPlacingObstructionEditorComponent obstructionComponent = SCR_CampaignBuildingPlacingObstructionEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingPlacingObstructionEditorComponent, true, true));
		if (!obstructionComponent)
		{
			outNotification = ENotification.EDITOR_PLACING_BLOCKED;
			return false;
		}

		if (obstructionComponent.IsPreviewOutOfRange(m_InstantPlacingParam, outNotification))
		{
			previewStateToShow = SCR_EPreviewState.BLOCKED;
			return false;
		}

		SCR_CampaignBuildingPlacingHQBaseEditorComponent hqBaseEditorComponent = SCR_CampaignBuildingPlacingHQBaseEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingPlacingHQBaseEditorComponent, true, true));
		if (hqBaseEditorComponent && hqBaseEditorComponent.IsNearAnyHQ(outNotification))
		{
			previewStateToShow = SCR_EPreviewState.BLOCKED;
			return false;
		}

		return obstructionComponent.CanCreate(outNotification, previewStateToShow);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		InitVariables();
		SetInitialEvent();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivateServer()
	{
		InitVariables();
		GetOnPlaceEntityServer().Insert(OnPlaceEntityServer);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpen()
	{
		super.EOnEditorOpen();
		InitVariables();
		GetOnPlaceEntity().Insert(GetCooldownTimeClient);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorClose()
	{
		super.EOnEditorClose();
		GetOnPlaceEntity().Remove(GetCooldownTimeClient);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpenServer()
	{
		super.EOnEditorOpenServer();
		InitVariables();
	}

	//------------------------------------------------------------------------------------------------
	private void SetInitialEvent()
	{
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(FindEditorComponent(SCR_PreviewEntityEditorComponent, true, true));
		if (!m_PreviewManager)
			return;

		m_PreviewManager.GetOnPreviewCreate().Insert(OnPreviewCreated);
	}

	//------------------------------------------------------------------------------------------------
	private void InitVariables()
	{
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));
		if (!m_CampaignBuildingComponent)
			return;

		m_Provider = m_CampaignBuildingComponent.GetProviderEntity();
		m_AreaTrigger = m_CampaignBuildingComponent.GetTrigger();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorDeactivateServer()
	{
		GetOnPlaceEntityServer().Remove(OnPlaceEntityServer);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the cooldown is set with this provider.
	bool UseCooldown()
	{
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(m_Provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return false;
		
		return providerComponent.IsBudgetToEvaluate(EEditableEntityBudget.COOLDOWN);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCooldownTimer(int cooldownTime)
	{
		SCR_EditorManagerEntity managerEnt = GetManager();
		if (!managerEnt)
			return;
		
		int playerId = managerEnt.GetPlayerID();
		
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(m_Provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;
		
		providerComponent.SetPlayerCooldown(playerId, cooldownTime);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAIBudget(int value)
	{		
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(m_Provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;
		
		providerComponent.AddAIValue(value);
	}
}

enum ECantBuildNotificationType
{
	OUT_OF_AREA = 0,
	BLOCKED = 1,
}
