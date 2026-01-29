void ScriptInvokerEntitySpawnedByProviderMethod(int prefabID, SCR_EditableEntityComponent editableEntity, int playerId, SCR_CampaignBuildingProviderComponent provider);
typedef func ScriptInvokerEntitySpawnedByProviderMethod;
typedef ScriptInvokerBase<ScriptInvokerEntitySpawnedByProviderMethod> ScriptInvokerEntitySpawnedByProvider;

void ScriptInvokerCompositionUnregisteredMethod(SCR_CampaignBuildingCompositionComponent composition);
typedef func ScriptInvokerCompositionUnregisteredMethod;
typedef ScriptInvokerBase<ScriptInvokerCompositionUnregisteredMethod> ScriptInvokerCompositionUnregistered;

[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Base for gamemode scripted component.")]
class SCR_CampaignBuildingManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

//! Interface for game mode extending components.
//! Must be attached to a GameMode entity.
class SCR_CampaignBuildingManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab of trigger spawned on server to activate a building mode when player enters its range.", "et")]
	protected ResourceName m_sFreeRoamBuildingServerTrigger;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab of trigger spawned only on clients, to visualize the building area and allow player build only within its radius.", "et")]
	protected ResourceName m_sFreeRoamBuildingClientTrigger;

	[Attribute("{58F07022C12D0CF5}Assets/Editor/PlacingPreview/Preview.emat", UIWidgets.ResourcePickerThumbnail, "Material used when for the preview of the compositions outside of the building mode.", category: "Preview", params: "emat")]
	protected ResourceName m_sPreviewMaterial;

	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	protected EEditableEntityBudget m_BudgetType;

	[Attribute("25", UIWidgets.EditBox, "Refund percentage", "")]
	protected int m_iCompositionRefundPercentage;

	[Attribute("10", UIWidgets.EditBox, "How many times player has to perform build step to gain a XP reward", "")]
	protected int m_iXpRewardTreshold;

	[Attribute("1", UIWidgets.EditBox, "How many times player has to perform building step at the beginning of gradual reward process to gain his first XP reward.", params: "1 inf 1")]
	protected int m_iInitialRewardInterval;

	[Attribute("0.2", UIWidgets.EditBox, "How steep curve of the gradual reward process will be.", params: "0 inf 0.001")]
	protected float m_fRewardCurveIncrement;

	//! Note: Provider is saved to composition only when it's built from base.
	[Attribute("0", UIWidgets.CheckBox, "If checked, only players of faction that match the owning faction of provider can disassemble composition")]
	protected bool m_bSameFactionDisassembleOnly;

	[Attribute("1", UIWidgets.CheckBox, "If checked, players from the opposing faction will only be able to disassemble composition when the base it belongs is being captured")]
	protected bool m_bDisassembleOnlyWhenCapturing;

	[Attribute()]
	protected ref SCR_CampaignBuildingCompositionOutlineManager m_OutlineManager;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Config with prefabs available to build. The config has to be the same as on Editor Mode - placing editor component..", params: "conf")]
	protected ResourceName m_sPrefabsToBuildResource;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, desc: "This prefab will be spawned when composition is disassembled but has no provider where to return supply.", params: "et")]
	protected ResourceName m_sPrefabToHoldSupplyOnRefund;

	protected ref array<ResourceName> m_aPlaceablePrefabs = {};
	protected ref map<SCR_CampaignMilitaryBaseComponent, ref array<SCR_CampaignBuildingCompositionComponent>> m_mCampaignBuildingComponents;
	protected ref array<SCR_CampaignBuildingCompositionComponent> m_aHQBuildingCompositions = {};

	protected SCR_EditableEntityCore m_EntityCore;
	protected IEntity m_TemporaryProvider;
	protected RplComponent m_RplComponent;
	protected int m_iBuildingCycle;
	protected float m_fRewardCurveProgress;

	protected ref ScriptInvokerEntitySpawnedByProvider m_OnEntitySpawnedByProvider;
	protected ref ScriptInvokerVoid m_OnAnyCompositionSpawned;
	protected ref ScriptInvokerCompositionUnregistered m_OnCompositionUnregistered;

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CampaignBuildingCompositionOutlineManager GetOutlineManager()
	{
		return m_OutlineManager;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetResourceComponent(IEntity owner, out SCR_ResourceComponent component)
	{
		if (!owner)
			return false;

		IEntity providerEntity;
		SCR_CampaignBuildingCompositionComponent campaignCompositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(owner.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (campaignCompositionComponent)
		providerEntity = campaignCompositionComponent.GetProviderEntity();

		if (!providerEntity)
		{
			providerEntity = GetTemporaryProvider();
			SetTemporaryProvider(null);
		}

		if (!providerEntity)
			return false;

		SetTemporaryProvider(null);

		component = SCR_ResourceComponent.FindResourceComponent(providerEntity);

		return component;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetCompositionPreviewMaterial()
	{
		return m_sPreviewMaterial;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetServerTriggerResourceName()
	{
		return m_sFreeRoamBuildingServerTrigger;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetClientTriggerResourceName()
	{
		return m_sFreeRoamBuildingClientTrigger;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] ent
	void SetTemporaryProvider(IEntity ent)
	{
		m_TemporaryProvider = ent;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetTemporaryProvider()
	{
		return m_TemporaryProvider;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool CanDisassembleSameFactionOnly()
	{
		return m_bSameFactionDisassembleOnly;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Can the enemy faction disassemble base structures ONLY while capturing.
	bool CanDisassembleOnlyWhenCapturing()
	{
		return m_bDisassembleOnlyWhenCapturing;
	}

	//------------------------------------------------------------------------------------------------
	//! Get supply component of the provider entity.
	[Obsolete("SCR_CampaignBuildingManagerComponent.GetResourceComponent() should be used instead.")]
	protected bool GetSupplyComponent(IEntity ownerEntity, out SCR_CampaignSuppliesComponent suppliesComponent)
	{
		SCR_CampaignBuildingCompositionComponent campaignCompositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(ownerEntity.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!campaignCompositionComponent)
			return false;

		IEntity providerEntity;
		providerEntity = campaignCompositionComponent.GetProviderEntity();
		if (!providerEntity)
		{
			providerEntity = GetTemporaryProvider();
			SetTemporaryProvider(null);
		}
		else
			SetTemporaryProvider(null);

		if (!providerEntity)
			return false;

		suppliesComponent = SCR_CampaignSuppliesComponent.Cast(providerEntity.FindComponent(SCR_CampaignSuppliesComponent));
		return suppliesComponent != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the number of cycles reaches a defined limit. If so, restart counter and send an RPC on server to add XP reward.
	void ProcesXPreward()
	{
		m_iBuildingCycle++;

		int currentThreshold = Math.Round(Math.Lerp(m_iInitialRewardInterval, m_iXpRewardTreshold, m_fRewardCurveProgress));
		if (m_iBuildingCycle < currentThreshold)
			return;

		m_iBuildingCycle = 0;

		m_fRewardCurveProgress += m_fRewardCurveIncrement;
		m_fRewardCurveProgress = Math.Clamp(m_fRewardCurveProgress, 0, 1);

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_CampaignBuildingNetworkComponent networkComponent = SCR_CampaignBuildingNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignBuildingNetworkComponent));
		if (!networkComponent)
			return;

		networkComponent.AddXPReward(playerController.GetPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	//! Get supplies from the composition cost that gets refunded on removal of composition
	//! \return Percentage of supplies refunded
	int GetCompositionRefundPercentage()
	{
		return m_iCompositionRefundPercentage;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntityCoreBudgetUpdated(EEditableEntityBudget entityBudget, int originalBudgetValue, int budgetChange, int updatedBudgetValue, SCR_EditableEntityComponent entity)
	{
		if (IsProxy())
			return;

		if (entityBudget != m_BudgetType)
			return;

		// Continue with compositions placed in WB only when refund is about to happen.
		if (entity.GetOwner().IsLoaded() && budgetChange > 0)
			return;

		//CampaignBuildingManagerComponent should not do anything if there is no campaign
        const SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		// Do not react to changes during loading of session
		if (SCR_PersistenceSystem.IsLoadInProgress())
			return; 

		int propBudgetValue;
		array<ref SCR_EntityBudgetValue> budgets = {};
		entity.GetEntityAndChildrenBudgetCost(budgets);

		//get props budget value
		foreach (SCR_EntityBudgetValue budget : budgets)
		{
			if (budget.GetBudgetType() != EEditableEntityBudget.PROPS)
				continue;

			propBudgetValue = budget.GetBudgetValue();
			break;
		}

		IEntity entityOwner = entity.GetOwnerScripted();
		SCR_ResourceComponent resourceComponent;
		bool wasContainerSpawned;

		// If resource component was not found on deconstruction, spawn a custom one , find again the resource component at this spawned box and fill it with refund supply.
		if (!GetResourceComponent(entityOwner, resourceComponent))
		{
			//Spawn a resource holder only when the refunded object is a composition.
			SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(entityOwner.FindComponent(SCR_CampaignBuildingCompositionComponent));
			if (compositionComponent && budgetChange < 0)
			{
				SpawnCustomResourceHolder(entityOwner, resourceComponent);
				wasContainerSpawned = true;
			}
		}

		if (!resourceComponent)
			return;

		//~ Supplies not enabled so no need to remove any
		if (!resourceComponent.IsResourceTypeEnabled())
			return;

		IEntity providerEntity = resourceComponent.GetOwner();

		if (!providerEntity)
			return;

		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(providerEntity.FindComponent(SCR_CampaignBuildingProviderComponent));

		if (budgetChange < 0)
		{
			budgetChange = Math.Round(budgetChange * m_iCompositionRefundPercentage * 0.01);

			if (providerComponent)
				providerComponent.AddPropValue(-propBudgetValue);

			if (wasContainerSpawned)
			{
				SCR_ResourceContainer container = resourceComponent.GetContainer(EResourceType.SUPPLIES);

				if (container)
					container.SetResourceValue(-budgetChange);
			}
			else
			{
				SCR_ResourceGenerator generator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

				if (generator)
					generator.RequestGeneration(-budgetChange);
			}
		}
		else
		{
			if (providerComponent)
				providerComponent.AddPropValue(propBudgetValue);

			SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

			if (consumer)
				consumer.RequestConsumtion(budgetChange);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SpawnCustomResourceHolder(IEntity entityOwner, out SCR_ResourceComponent component)
	{
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		entityOwner.GetTransform(spawnParams.Transform);

		Resource res = Resource.Load(m_sPrefabToHoldSupplyOnRefund);
		if (!res.IsValid())
			return;

		IEntity resourceHolder = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), spawnParams);
		if (!resourceHolder)
			return;

		component = SCR_ResourceComponent.FindResourceComponent(resourceHolder);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		GetPrefabListFromConfig();

		if (!GetGameMode().IsMaster())
			return;

		// Supplies are disabled in this mode. No need for an event.
		if (!SCR_ResourceSystemHelper.IsGlobalResourceTypeEnabled(EResourceType.SUPPLIES))
			return;

		m_EntityCore.Event_OnEntityBudgetUpdatedPerEntity.Insert(OnEntityCoreBudgetUpdated);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	//! Load all the available compositions at the beginning of the game to be accessible without the building mode entity.
	void GetPrefabListFromConfig()
	{
		Resource configContainer = BaseContainerTools.LoadContainer(m_sPrefabsToBuildResource);
		if (!configContainer || !configContainer.IsValid())
			return;

		SCR_PlaceableEntitiesRegistry registry = SCR_PlaceableEntitiesRegistry.Cast(BaseContainerTools.CreateInstanceFromContainer(configContainer.GetResource().ToBaseContainer()));
		if (!registry)
			return;

		m_aPlaceablePrefabs = registry.GetPrefabs();
	}

	//------------------------------------------------------------------------------------------------
	//! Search for a resource name of composition by given ID.
	//! \param[in] prefabID
	//! \return
	ResourceName GetCompositionResourceName(int prefabID)
	{
		// the array doesn't exist or the index I'm searching for is out of the bounds, terminate.
		if (!m_aPlaceablePrefabs || !m_aPlaceablePrefabs.IsIndexValid(prefabID))
			return string.Empty;

		return m_aPlaceablePrefabs[prefabID];
	}

	//------------------------------------------------------------------------------------------------
	//! Returns composition id based on provided resource name.
	//! \param[in] resName
	//! \return
	int GetCompositionId(ResourceName resName)
	{
		return m_aPlaceablePrefabs.Find(resName);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \param[in] provider
	//! \param[in] userActionActivationOnly
	//! \param[in] userActionUsed
	void GetEditorMode(int playerID, notnull IEntity provider, bool userActionActivationOnly = false, bool userActionUsed = false)
	{
		SCR_EditorManagerEntity editorManager = GetEditorManagerEntity(playerID);
		if (!editorManager)
			return;

		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!modeEntity)
			modeEntity = editorManager.CreateEditorMode(EEditorMode.BUILDING, false);

		if (!modeEntity)
			return;

		SetEditorMode(editorManager, modeEntity, playerID, provider, userActionActivationOnly, userActionUsed);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEditorMode(notnull SCR_EditorManagerEntity editorManager, notnull SCR_EditorModeEntity modeEntity, int playerID, notnull IEntity provider, bool userActionActivationOnly = false, bool userActionUsed = false)
	{
		SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingComponent)
			return;

		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;

		providerComponent.AddNewAvailableUser(playerID);
		if (userActionUsed)
			providerComponent.AddNewActiveUser(playerID);

		buildingComponent.AddProviderEntityEditorComponent(providerComponent);

		if (!editorManager.IsOpened())
			editorManager.SetCurrentMode(EEditorMode.BUILDING);

		if (userActionActivationOnly || userActionUsed)
			ToggleEditorMode(editorManager);

		// events
		SCR_PlacingEditorComponent placingComponent = SCR_PlacingEditorComponent.Cast(modeEntity.FindComponent(SCR_PlacingEditorComponent));
		placingComponent.GetOnPlaceEntityServer().Insert(EntitySpawnedByProvider);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDisconnected().Insert(PlayerDisconnected);

		SetOnPlayerDeathEvent(playerID);
		SetOnProviderDestroyedEvent(provider);
		providerComponent.SetCheckProviderMove();
	}

	//------------------------------------------------------------------------------------------------
	//! Event raised when the player disconnects
	//! \param[in] playerId
	//! \param[in] cause
	//! \param[in] timeout
	void PlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		gameMode.GetOnPlayerDisconnected().Remove(PlayerDisconnected);
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggered when the entity is spawned by this provider.
	void EntitySpawnedByProvider(int prefabID, SCR_EditableEntityComponent editableEntity, int playerId)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		SCR_EditorManagerEntity editorManager = GetEditorManagerEntity(playerId);
		if (!editorManager)
			return;

		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!modeEntity)
			return;

		SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingComponent)
			return;

		SCR_CampaignBuildingProviderComponent provider = buildingComponent.GetProviderComponent();
		if (!provider)
			return;

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!player)
			return;

		campaign.OnEntityRequested(editableEntity.GetOwner(), player, SCR_Faction.Cast(SCR_Faction.GetEntityFaction(editableEntity.GetOwner())), provider);

		if (m_OnEntitySpawnedByProvider)
			m_OnEntitySpawnedByProvider.Invoke(prefabID, editableEntity, playerId, provider);
	}

	//------------------------------------------------------------------------------------------------
	protected bool TryAddHQ(notnull SCR_CampaignBuildingCompositionComponent composition)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(composition.GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!editableEntity)
			return false;

		if (!IsHQService(editableEntity))
			return false;

		m_aHQBuildingCompositions.Insert(composition);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool TryRemoveHQ(notnull SCR_CampaignBuildingCompositionComponent composition)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(composition.GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!editableEntity)
			return false;

		if (!IsHQService(editableEntity))
			return false;

		m_aHQBuildingCompositions.RemoveItem(composition);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsHQService(notnull SCR_EditableEntityComponent editableEntity)
	{
		SCR_EditableEntityUIInfo prefabInfo = SCR_EditableEntityUIInfo.Cast(editableEntity.GetInfo());
		if (!prefabInfo)
			return false;

		array<EEditableEntityLabel> entityLabels = {};
		prefabInfo.GetEntityLabels(entityLabels);
		return entityLabels.Contains(EEditableEntityLabel.SERVICE_HQ);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetOnPlayerDeathEvent(int playerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return;

		SCR_CharacterControllerComponent comp = SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
		if (!comp)
			return;

		comp.GetOnPlayerDeathWithParam().Insert(OnPlayerDeath);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveOnPlayerDeathEvent(int playerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return;

		SCR_CharacterControllerComponent comp = SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
		if (!comp)
			return;

		comp.GetOnPlayerDeathWithParam().Remove(OnPlayerDeath);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetOnProviderDestroyedEvent(IEntity provider)
	{
		SCR_DamageManagerComponent hitZoneContainerComponent = SCR_DamageManagerComponent.Cast(provider.FindComponent(SCR_DamageManagerComponent));
		if (!hitZoneContainerComponent)
			return;

		SCR_HitZone zone = SCR_HitZone.Cast(hitZoneContainerComponent.GetDefaultHitZone());
		if (zone)
			zone.GetOnDamageStateChanged().Insert(OnProviderDestroyed);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveOnProviderDestroyedEvent(IEntity provider)
	{
		SCR_DamageManagerComponent hitZoneContainerComponent = SCR_DamageManagerComponent.Cast(provider.FindComponent(SCR_DamageManagerComponent));
		if (!hitZoneContainerComponent)
			return;

		SCR_HitZone zone = SCR_HitZone.Cast(hitZoneContainerComponent.GetDefaultHitZone());
		if (zone)
			zone.GetOnDamageStateChanged().Remove(OnProviderDestroyed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerDeath(SCR_CharacterControllerComponent characterController, IEntity instigatorEntity, notnull Instigator killer)
	{
		if (!characterController)
			return;

		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(characterController.GetOwner());

		SCR_EditorManagerEntity editorManager = GetEditorManagerEntity(playerID);
		if (!editorManager)
			return;

		if (editorManager.IsOpened())
			ToggleEditorMode(editorManager);

		RemoveEditorMode(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the provider was destroyed.
	//! \param[in] hitZone
	protected void OnProviderDestroyed(SCR_HitZone hitZone)
	{
		if (!hitZone)
			return;

		if (hitZone.GetDamageState() != EDamageState.DESTROYED)
			return;

		IEntity provider = hitZone.GetOwner();
		if (!provider)
			return;

		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;

		array<int> playersIDs = {};
		providerComponent.GetAvailableUsers(playersIDs);
		foreach (int playerId : playersIDs)
		{
			if (providerComponent.ContainActiveUsers(playerId))
			{
				RemoveProvider(playerId, providerComponent, true);
				providerComponent.RemoveActiveUser(playerId);
			}

			providerComponent.RemoveAvailableUser(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Remove the provided player id from list of active and available users, return true if the user was in the list of active users
	//! \param[in] playerID
	//! \param[in] providerComponent
	//! \return
	bool RemovePlayerIdFromProvider(int playerID, SCR_CampaignBuildingProviderComponent providerComponent)
	{
		bool isActiveUser = providerComponent.ContainActiveUsers(playerID);

		providerComponent.RemoveActiveUser(playerID);
		providerComponent.RemoveAvailableUser(playerID);

		return isActiveUser;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] providerComponent
	//! \param[in] isActiveUser
	void RemoveProvider(int playerID, SCR_CampaignBuildingProviderComponent providerComponent, bool isActiveUser)
	{
		SCR_EditorManagerEntity editorManager = GetEditorManagerEntity(playerID);
		if (!editorManager)
			return;

		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!modeEntity)
			return;

		SCR_CampaignBuildingEditorComponent editorComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!editorComponent)
			return;

		if (isActiveUser && editorManager.IsOpened())
			editorManager.Close();

		editorComponent.RemoveProviderEntityEditorComponent(providerComponent);

		IEntity provider = providerComponent.GetOwner();
		if (provider)
			RemoveOnProviderDestroyedEvent(providerComponent.GetOwner());

		providerComponent.RemoveCheckProviderMove();
		RemoveOnPlayerDeathEvent(playerID);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDisconnected().Remove(PlayerDisconnected);

		// if it was a last provider and forced provider doesn't exist, remove the mode completely.
		if (!editorComponent.GetProviderComponent())
			RemoveEditorMode(playerID);
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleEditorMode(notnull SCR_EditorManagerEntity editorManager)
	{
		EEditorMode mode = editorManager.GetCurrentMode();

		editorManager.Toggle();
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveEditorMode(int playerID)
	{
		SCR_EditorManagerEntity editorManager = GetEditorManagerEntity(playerID);
		if (!editorManager)
			return;

		SCR_EditorModeEntity editorModeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!editorModeEntity)
			return;

		SCR_EntityHelper.DeleteEntityAndChildren(editorModeEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_EditorManagerEntity GetEditorManagerEntity(int playerID)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		return core.GetEditorManager(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] composition
	void RegisterComposition(notnull SCR_CampaignBuildingCompositionComponent composition)
	{
		if (!m_mCampaignBuildingComponents)
			m_mCampaignBuildingComponents = new map<SCR_CampaignMilitaryBaseComponent, ref array<SCR_CampaignBuildingCompositionComponent>>();

		SCR_MilitaryBaseSystem baseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (!baseSystem)
			return;

		TryAddHQ(composition);

		vector position = composition.GetOwner().GetOrigin();
		array<SCR_MilitaryBaseComponent> bases = {};
		baseSystem.GetBases(bases);

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
			if (!campaignBase)
				continue;

			if (vector.DistanceSqXZ(campaignBase.GetOwner().GetOrigin(), position) <= (campaignBase.GetRadius() * campaignBase.GetRadius()))
			{
				if (!m_mCampaignBuildingComponents.Contains(campaignBase))
				{
					m_mCampaignBuildingComponents.Set(campaignBase, new array<SCR_CampaignBuildingCompositionComponent>());
				}

				array<SCR_CampaignBuildingCompositionComponent> baseComponents = m_mCampaignBuildingComponents[campaignBase];
				if (!baseComponents.Contains(composition))
				{
					baseComponents.Insert(composition);
					composition.GetOnCompositionSpawned().Insert(OnCompositionSpawned);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] composition
	void UnregisterComposition(notnull SCR_CampaignBuildingCompositionComponent composition)
	{
		if (!m_mCampaignBuildingComponents)
			return;

		SCR_MilitaryBaseSystem baseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (!baseSystem)
			return;

		TryRemoveHQ(composition);

		array<SCR_MilitaryBaseComponent> bases = {};
		baseSystem.GetBases(bases);

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
			if (!campaignBase)
				continue;

			array<SCR_CampaignBuildingCompositionComponent> baseComponents = m_mCampaignBuildingComponents[campaignBase];
			if (!baseComponents)
				continue;

			if (baseComponents.Contains(composition))
			{
				baseComponents.RemoveItem(composition);
				composition.GetOnCompositionSpawned().Remove(OnCompositionSpawned);

				if (m_OnCompositionUnregistered)
					m_OnCompositionUnregistered.Invoke(composition);

				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompositionSpawned(bool spawned)
	{
		if (!spawned)
			return;

		if (m_OnAnyCompositionSpawned)
			m_OnAnyCompositionSpawned.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	int GetBuildingCompositions(SCR_CampaignMilitaryBaseComponent base, out array<SCR_CampaignBuildingCompositionComponent> compositions)
	{
		if (!m_mCampaignBuildingComponents)
			return 0;

		array<SCR_CampaignBuildingCompositionComponent> baseComponents = m_mCampaignBuildingComponents[base];
		if (!baseComponents)
			return 0;

		return compositions.Copy(baseComponents);
	}

	//------------------------------------------------------------------------------------------------
	//! \return spawned HQ building compositions
	array<SCR_CampaignBuildingCompositionComponent> GetHQBuildingCompositions()
	{
		return m_aHQBuildingCompositions;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerEntitySpawnedByProvider GetOnEntitySpawnedByProvider()
	{
		if (!m_OnEntitySpawnedByProvider)
			m_OnEntitySpawnedByProvider = new ScriptInvokerEntitySpawnedByProvider();

		return m_OnEntitySpawnedByProvider;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnAnyCompositionSpawned()
	{
		if (!m_OnAnyCompositionSpawned)
			m_OnAnyCompositionSpawned = new ScriptInvokerVoid();

		return m_OnAnyCompositionSpawned;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerCompositionUnregistered GetOnCompositionUnregistered()
	{
		if (!m_OnCompositionUnregistered)
			m_OnCompositionUnregistered = new ScriptInvokerCompositionUnregistered();

		return m_OnCompositionUnregistered;
	}
}
