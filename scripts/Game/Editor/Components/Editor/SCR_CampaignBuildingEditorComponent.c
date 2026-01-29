// Attached on EditorModeBuilding prefab
[ComponentEditorProps(category: "GameScripted/Editor", description: "Main conflict component for handling building editor mode", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingEditorComponentClass : SCR_BaseEditorComponentClass
{
}

//------------------------------------------------------------------------------------------------

class SCR_CampaignBuildingEditorComponent : SCR_BaseEditorComponent
{
	protected ref array<SCR_CampaignBuildingProviderComponent> m_aProvidersComponents = {};
	protected ref array<RplId> m_aProvidersRplIds = {};
	protected SCR_ContentBrowserEditorComponent m_ContentBrowserManager;
	protected SCR_CampaignBuildingProviderComponent m_ForcedProviderComponent;
	protected bool m_bViewObstructed;
	protected ScriptedGameTriggerEntity m_BuildingAreaTrigger;

	protected ref ScriptInvoker m_OnProviderChanged;
	protected ref ScriptInvokerBool m_OnObstructionEventTriggered;

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnProviderChanged()
	{
		if (!m_OnProviderChanged)
			m_OnProviderChanged = new ScriptInvoker();

		return m_OnProviderChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnObstructionEventTriggered()
	{
		if (!m_OnObstructionEventTriggered)
			m_OnObstructionEventTriggered = new ScriptInvokerBool();

		return m_OnObstructionEventTriggered;
	}

	//------------------------------------------------------------------------------------------------
	void AddProviderEntityEditorComponent(SCR_CampaignBuildingProviderComponent providerComponent)
	{
		if (!providerComponent || m_aProvidersComponents.Contains(providerComponent))
			return;

		SCR_MilitaryBaseComponent base = providerComponent.GetMilitaryBaseComponent();
		if (base)
			m_aProvidersComponents.InsertAt(providerComponent, 0);
		else
			m_aProvidersComponents.Insert(providerComponent);

		GetOnProviderChanged().Invoke(m_aProvidersComponents[0]);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveProviderEntityEditorComponent(SCR_CampaignBuildingProviderComponent providerComponent)
	{
		if (m_ForcedProviderComponent == providerComponent)
			SetForcedProvider(null);

		m_aProvidersComponents.RemoveItemOrdered(providerComponent);

		if (!m_aProvidersComponents.IsEmpty())
			GetOnProviderChanged().Invoke(m_aProvidersComponents[0]);
	}

	//------------------------------------------------------------------------------------------------
	//! Return provider component of current provider.
	SCR_CampaignBuildingProviderComponent GetProviderComponent(bool getMasterProviderComponent = false)
	{
		SCR_CampaignBuildingProviderComponent temporaryProviderComponent;
		if (m_ForcedProviderComponent)
			temporaryProviderComponent = m_ForcedProviderComponent;

		if (m_aProvidersComponents.IsEmpty())
			return null;

		temporaryProviderComponent = m_aProvidersComponents[0];
		
		if (getMasterProviderComponent && temporaryProviderComponent && temporaryProviderComponent.UseMasterProvider())
			return SCR_CampaignBuildingProviderComponent.Cast(GetMasterProviderEntity().FindComponent(SCR_CampaignBuildingProviderComponent));
		
		return temporaryProviderComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used when player initiate a building mode via user action - forced provider is an entity owning the user action
	void SetForcedProvider(SCR_CampaignBuildingProviderComponent forcedProviderComponent = null)
	{
		m_ForcedProviderComponent = forcedProviderComponent;
	}

	//------------------------------------------------------------------------------------------------
	int GetProviderEntitiesCount()
	{
		return m_aProvidersComponents.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool IsActiveProvider(notnull SCR_CampaignBuildingProviderComponent providerComponent)
	{
		if (providerComponent == m_ForcedProviderComponent)
			return true;

		// there exist a forced provider and it's not a tested entity, then it can't be an active provider. ToDo: Zamyslet se a zeptat
		if (m_ForcedProviderComponent)
			return false;

		if (!m_aProvidersComponents.IsEmpty() && m_aProvidersComponents[0] == providerComponent)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool GetProviderResourceComponent(out SCR_ResourceComponent resourceComponent)
	{
		if (m_aProvidersComponents.IsEmpty())
			return false;

		resourceComponent = SCR_ResourceComponent.FindResourceComponent(GetProviderEntity());
		
		if (resourceComponent)
			return true;

		IEntity parent = GetProviderEntity();
		
		while (parent)
		{
			resourceComponent = SCR_ResourceComponent.FindResourceComponent(parent);
			
			if (resourceComponent)
				return true;

			parent = parent.GetParent();
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check current provider and return budget marked as a budget to be shown in UI progress bar. If more budgets are marked, the 1st one found is used.
	EEditableEntityBudget GetShownBudget()
	{
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return -1;
		
		return providerComponent.GetShownBudget();
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("SCR_CampaignBuildingEditorComponent.GetProviderResourceComponent() should be used instead.")]
	bool GetProviderSuppliesComponent(out SCR_CampaignSuppliesComponent suppliesComponent)
	{
		if (m_aProvidersComponents.IsEmpty())
			return false;

		suppliesComponent = SCR_CampaignSuppliesComponent.Cast(GetProviderEntity().FindComponent(SCR_CampaignSuppliesComponent));
		if (suppliesComponent)
			return true;

		IEntity parent = GetProviderEntity();
		while (parent)
		{
			suppliesComponent = SCR_CampaignSuppliesComponent.Cast(parent.FindComponent(SCR_CampaignSuppliesComponent));
			if (suppliesComponent)
				return true;

			parent = parent.GetParent();
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	SCR_FactionAffiliationComponent GetProviderFactionComponent()
	{
		if (m_aProvidersComponents.IsEmpty())
			return null;

		SCR_FactionAffiliationComponent factionComp = SCR_FactionAffiliationComponent.Cast(GetProviderEntity().FindComponent(SCR_FactionAffiliationComponent));
		if (factionComp)
			return factionComp;

		IEntity parent = GetProviderEntity();
		while (parent)
		{
			factionComp = SCR_FactionAffiliationComponent.Cast(parent.FindComponent(SCR_FactionAffiliationComponent));
			if (factionComp)
				return factionComp;

			parent = parent.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	ScriptedGameTriggerEntity GetTrigger()
	{		
		return m_BuildingAreaTrigger;
	}

	//------------------------------------------------------------------------------------------------
	//~  This function will add/remove the faction label of the saved editor state. Thus displaying the correct faction entities in the menu
	protected void AddRemoveFactionLabel(SCR_Faction faction, bool addLabel)
	{
		if (!faction)
			return;

		m_ContentBrowserManager.AddRemoveLabelOfPersistentBrowserState(faction.GetFactionLabel(), addLabel);
	}

	//------------------------------------------------------------------------------------------------
	ScriptedGameTriggerEntity SpawnClientTrigger()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return null;

		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return null;
		
		IEntity provider = GetProviderEntity(providerComponent.UseMasterProvider());
		if (!provider)
			return null;

		Resource resource = Resource.Load(buildingManagerComponent.GetClientTriggerResourceName());
		if (!resource || !resource.IsValid())
			return null;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Parent = provider;

		return ScriptedGameTriggerEntity.Cast(GetGame().SpawnEntityPrefab(resource, provider.GetWorld(), params));
	}

	//---- REFACTOR NOTE START: Has a hot fix for content browser as for some reason it would show an empty entry. So we skip an entry from the catalog and simply hide it ----
	//------------------------------------------------------------------------------------------------
	//! Make the area around where is possible to build composition visible for player
	override protected void EOnEditorActivate()
	{
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent(true);
		if (!providerComponent)
			return;
		
		if (!System.IsConsoleApp() && GetGame().GetPlayerController())
		{
			m_BuildingAreaTrigger = SpawnClientTrigger();

			if (m_BuildingAreaTrigger)
			{
				m_BuildingAreaTrigger.SetSphereRadius(providerComponent.GetBuildingRadius());

				SCR_CampaignBuildingAreaMeshComponent areaMeshComponent = SCR_CampaignBuildingAreaMeshComponent.Cast(m_BuildingAreaTrigger.FindComponent(SCR_CampaignBuildingAreaMeshComponent));
				if (areaMeshComponent && areaMeshComponent.ShouldEnableFrameUpdateDuringEditor())
				{
					areaMeshComponent.ActivateEveryFrame();
					areaMeshComponent.GenerateAreaMesh();
				}

				m_BuildingAreaTrigger.SetFlags(EntityFlags.VISIBLE, false);
			}
		}
		
		if (providerComponent.ObstrucViewWhenEnemyInRange())
			SetOnEnterEvent();

		m_ContentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));

		FactionAffiliationComponent factionComponent = GetProviderFactionComponent();
		if (!factionComponent)
			return;

		Faction buildingFaction;
		if (SCR_VehicleFactionAffiliationComponent.Cast(factionComponent))
			buildingFaction = factionComponent.GetDefaultAffiliatedFaction();
		else
		{
			buildingFaction = factionComponent.GetAffiliatedFaction();

			if (!buildingFaction)
				buildingFaction = factionComponent.GetDefaultAffiliatedFaction();
		}

		if (buildingFaction)
			AddRemoveFactionLabel(SCR_Faction.Cast(buildingFaction), true);

		array<SCR_EditorContentBrowserSaveStateDataUI> contentBrowserStates = {};
		int tabsCount = m_ContentBrowserManager.GetContentBrowserTabStates(contentBrowserStates);

		for (int i = 0; i < tabsCount; i++)
		{
			if (!contentBrowserStates[i])
				continue;
			
			//~ Todo: Fix first tab being broken
			//~ Hotfix for first tab being broken
			if (i == 0 || !CanBeShown(contentBrowserStates[i]))
				m_ContentBrowserManager.SetStateTabVisible(i, false);
		}

		ToggleBuildingTool(false);

		SCR_CampaignBuildingProviderComponent nonMasterProviderComponent = GetProviderComponent(false);
		if (!nonMasterProviderComponent)
			return;

		if (!nonMasterProviderComponent.UseAllAvailableProviders())
			return;

		array<SCR_MilitaryBaseComponent> bases = {};
		int basesCount = nonMasterProviderComponent.GetBases(bases);
		if (basesCount > 0)
		{
			bases[0].GetOnMilitaryBaseRegistered().Insert(OnMilitaryBaseRegistrationChanged);
			bases[0].GetOnMilitaryBaseUnregistered().Insert(OnMilitaryBaseRegistrationChanged);
		}
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! Check if the given tab can be shown
	bool CanBeShown(notnull SCR_EditorContentBrowserSaveStateDataUI tab)
	{
		if (!TabContainLabel(tab))
			return false;

		return tab.CanBeShown();
	}
	
	//------------------------------------------------------------------------------------------------
	//! returns true if the view should be obsctructed
	bool IsViewObstructed()
	{
		return m_bViewObstructed;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the given tabUI contains any label set on provider.
	bool TabContainLabel(SCR_EditorContentBrowserSaveStateDataUI tab)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return false;

		array<EEditableEntityLabel> labels = providerComponent.GetAvailableTraits();

		foreach (EEditableEntityLabel label : labels)
		{
			if (tab.ContainsLabel(label))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if the provider is a base
	bool IsProviderBase()
	{
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return false;

		return providerComponent.GetMilitaryBaseComponent();
	}

	//------------------------------------------------------------------------------------------------
	void ToggleBuildingTool(bool mode)
	{
		IEntity player = EntityUtils.GetPlayer();
		if (!player)
			return;

		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(player);
		if (!gadgetManager)
			return;
		
		SCR_CampaignBuildingGadgetToolComponent gadgetComponent = SCR_CampaignBuildingGadgetToolComponent.Cast(gadgetManager.GetHeldGadgetComponent());
		if (!gadgetComponent)
			return;
				
		if (mode)
			gadgetComponent.ToolToHand();
		else
			gadgetComponent.ToolToInventory();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set event to obstruct view when enemy character enters a building radius
	protected void SetOnEnterEvent()
	{
		SCR_FreeRoamBuildingClientTriggerEntity trigger = SCR_FreeRoamBuildingClientTriggerEntity.Cast(GetTrigger());
		if (!trigger)
			return;
		
		trigger.GetOnEntityEnterTrigger().Insert(EntityEnterTrigger);
		trigger.GetOnEntityLeaveTrigger().Insert(EntityLeaveTrigger);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EntityEnterTrigger(IEntity ent)
	{
		if (CanBlockView(ent))
		{
			SetOnEntityKilled(ent);
			
			// show the notification and rise an event only when this is for the first time.
			if (!m_bViewObstructed && m_OnObstructionEventTriggered)
			{
				m_OnObstructionEventTriggered.Invoke(true);
				SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_ENEMY_IN_AREA);
				m_bViewObstructed = true;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Called when entity leaves the trigger. Can be also when the entity is killed or deleted.
	//! \param[in] ent entity removed from the trigger. Can be null when event is triggered from deleted or killed entity.
	protected void EntityLeaveTrigger(IEntity ent)
	{			
		if (!ent)
			return;
		
		if (CanBlockView(ent))
			RemoveOnEntityKilled(ent);
		
		ScriptedGameTriggerEntity trigger = GetTrigger();
		if (!trigger)
			return;
		
		trigger.QueryEntitiesInside();
		array<IEntity> entitiesInside = {};
		trigger.GetEntitiesInside(entitiesInside);
		
		// Check if in trigger is still present any entity that can block the view. If so, don't continue.
		foreach (IEntity entity : entitiesInside)
		{
			// The entity that triggers an event as the leaving one is still on the list of entities present in trigger, skip it.
			if (ent == entity)
				continue;
			
			if (entity && CanBlockView(entity))
				return;
		}
		
		if (m_OnObstructionEventTriggered)
			m_OnObstructionEventTriggered.Invoke(false);
		
		m_bViewObstructed = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check the entity can block playeres view in Free Roam Building.
	//! \param[in] ent entity to evaluate
	protected bool CanBlockView(notnull IEntity ent)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character)
			return false;
		
		CharacterControllerComponent charControl = character.GetCharacterController();
		if (!charControl || charControl.GetLifeState() == ECharacterLifeState.DEAD)
			return false;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		if (playerId == 0)
		{			
			AIControlComponent ctrComp = charControl.GetAIControlComponent();
			if (ctrComp && !ctrComp.IsAIActivated())
				return false;
		}

		foreach (SCR_CampaignBuildingProviderComponent providerComponent : m_aProvidersComponents)
		{
			if (!providerComponent || !providerComponent.IsEnemyFaction(character))
				continue;

			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set an event called when entity which can block view in Free Roam Building enters the area of Free Roam Building.
	void SetOnEntityKilled(IEntity ent)
	{
		SCR_EditableCharacterComponent editableCharacter = SCR_EditableCharacterComponent.Cast(ent.FindComponent(SCR_EditableCharacterComponent));
		if (!editableCharacter)
			return;
		
		editableCharacter.GetOnDestroyed().Insert(EntityLeaveTrigger);
		editableCharacter.GetOnDeleted().Insert(EntityLeaveTrigger);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove an event called when entity which can block view in Free Roam Building is killed / leave the area of Free Roam Building.
	void RemoveOnEntityKilled(IEntity ent)
	{
		SCR_EditableCharacterComponent editableCharacter = SCR_EditableCharacterComponent.Cast(ent.FindComponent(SCR_EditableCharacterComponent));
		if (!editableCharacter)
			return;
		
		editableCharacter.GetOnDestroyed().Remove(EntityLeaveTrigger);
		editableCharacter.GetOnDeleted().Remove(EntityLeaveTrigger);
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetUserRank()
	{
		int playerId = SCR_PlayerController.GetLocalPlayerId();

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return SCR_ECharacterRank.INVALID;

		return SCR_CharacterRankComponent.GetCharacterRank(playerController.GetControlledEntity());
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpenServer()
	{
		SCR_MilitaryBaseComponent base;

		if (GetProviderEntity())
		{
			SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(GetProviderEntity().FindComponent(SCR_CampaignBuildingProviderComponent));
			if (!providerComponent)
				return;

			base = providerComponent.GetMilitaryBaseComponent();
		}

		if (base)
			return;

		GetGame().GetWorld().QueryEntitiesBySphere(GetProviderEntity().GetOrigin(), GetProviderComponent().GetBuildingRadius(), AssociateCompositionsToProvider, null, EQueryEntitiesFlags.ALL);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorCloseServer()
	{
		ToggleBuildingTool(true);

		if (!GetProviderEntity())
			return;

		// In case the provider of the building was the base, don't remove it's stamp from component. So the composition can't be build from another providers
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(GetProviderEntity().FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;

		SCR_MilitaryBaseComponent base = providerComponent.GetMilitaryBaseComponent();
		if (base)
			return;

		GetGame().GetWorld().QueryEntitiesBySphere(GetProviderEntity().GetOrigin(), GetProviderComponent().GetBuildingRadius(), UnassignCompositionProvider, null, EQueryEntitiesFlags.ALL);
	}

	//------------------------------------------------------------------------------------------------
	bool AssociateCompositionsToProvider(IEntity ent)
	{
		SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(ent.FindComponent(SCR_EditableEntityComponent));
		if (!editableComponent || editableComponent.GetParentEntity())
			return true;

		SCR_CampaignBuildingCompositionComponent comp = SCR_CampaignBuildingCompositionComponent.Cast(ent.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!comp)
			return true;

		// compositioni has no owner, set one.
		if (comp.GetProviderEntity() == null)
			comp.SetProviderEntityServer(GetProviderEntity());

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool UnassignCompositionProvider(IEntity ent)
	{
		SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(ent.FindComponent(SCR_EditableEntityComponent));
		if (!editableComponent || editableComponent.GetParentEntity())
			return true;

		SCR_CampaignBuildingCompositionComponent comp = SCR_CampaignBuildingCompositionComponent.Cast(ent.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!comp)
			return true;

		if (comp.GetProviderEntity() == GetProviderEntity())
			comp.SetProviderEntityServer(null);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Hide the area of building and remove the active faction
	override protected void EOnEditorDeactivate()
	{
		ScriptedGameTriggerEntity trigger = GetTrigger();
		if (trigger)
		{
			IEntity ent = trigger.GetParent();
			if (ent)
				ent.RemoveChild(trigger);
		
			trigger.Update();
			SCR_EntityHelper.DeleteEntityAndChildren(trigger);
		}

		FactionAffiliationComponent factionComponent = GetProviderFactionComponent();
		if (factionComponent)
		{
			// here we have to check both Affiliated faction and Default affiliated faction in case of vehicles. It's because vehicles can't have set a affiliated faction if no one is sitting inside (to prevent AI to shoot at empty vehicles)
			Faction buildingFaction = factionComponent.GetAffiliatedFaction();

			if (!buildingFaction)
				buildingFaction = factionComponent.GetDefaultAffiliatedFaction();

			if (buildingFaction)
				AddRemoveFactionLabel(SCR_CampaignFaction.Cast(buildingFaction), false);
		}
		
		ToggleBuildingTool(true);

		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent(false);
		if (!providerComponent)
			return;

		if (!providerComponent.UseAllAvailableProviders())
			return;

		array<SCR_MilitaryBaseComponent> bases = {};
		int basesCount = providerComponent.GetBases(bases);
		if (basesCount > 0)
		{
			bases[0].GetOnMilitaryBaseRegistered().Remove(OnMilitaryBaseRegistrationChanged);
			bases[0].GetOnMilitaryBaseUnregistered().Remove(OnMilitaryBaseRegistrationChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMilitaryBaseRegistrationChanged(SCR_MilitaryBaseLogicComponent baseComponent)
	{
		SCR_CampaignBuildingProviderComponent provider = SCR_CampaignBuildingProviderComponent.Cast(baseComponent);
		if (!provider)
			return;

		UpdateTabs();
	}

	//------------------------------------------------------------------------------------------------
	//! Update tabs
	void UpdateTabs()
	{
		if (!m_ContentBrowserManager)
			m_ContentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));

		array<SCR_EditorContentBrowserSaveStateDataUI> contentBrowserStates = {};
		int tabsCount = m_ContentBrowserManager.GetContentBrowserTabStates(contentBrowserStates);

		for (int i = 0; i < tabsCount; i++)
		{
			if (!contentBrowserStates[i])
				continue;

			if (i == 0 || !CanBeShown(contentBrowserStates[i]))
				m_ContentBrowserManager.SetStateTabVisible(i, false);
			else
				m_ContentBrowserManager.SetStateTabVisible(i, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Return the entity which belongs to currently active provider componenet.
	//! bool getMasterProvider default false. If true, tries to search for master provider entity.
	IEntity GetProviderEntity(bool getMasterProvider = false)
	{
		if (getMasterProvider)
			return GetMasterProviderEntity();
		
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return null;

		return providerComponent.GetOwner();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return the HQ entity of the base to which the current provider can be registered to. 
	//! For an example provider is a living area but this one is registered to a base so the base entity is returned. If the living area is standalone, the entity of living area is returned.
	IEntity GetMasterProviderEntity()
	{
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return null;
		
		return providerComponent.GetMasterProviderEntity();
	}

	//------------------------------------------------------------------------------------------------
	//! Set a provider, loaded from RPL ID
	void SetProviderFromRplID()
	{
		int count = m_aProvidersRplIds.Count();
		for (int i = 0; i < count; i++)
		{
			RplComponent rplComp = RplComponent.Cast(Replication.FindItem(m_aProvidersRplIds[i]));
			if (!rplComp)
				continue;

			IEntity provider = rplComp.GetEntity();
			if (!provider)
				continue;

			SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
			if (!providerComponent)
				continue;

			AddProviderEntityEditorComponent(providerComponent);
			m_aProvidersRplIds.RemoveOrdered(i);
			count--;
		}

		if (m_aProvidersRplIds.IsEmpty())
			SCR_CampaignBuildingProviderComponent.GetOnProviderCreated().Remove(SetProviderFromRplID);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RplSave(ScriptBitWriter writer)
	{
		RplId entityRplID;
		int count = m_aProvidersComponents.Count();

		writer.WriteInt(count);
		for (int i = 0; i < count; i++)
		{
			IEntity provider = m_aProvidersComponents[i].GetOwner();
			if (!provider)
				continue;

			RplComponent rplCmp = RplComponent.Cast(provider.FindComponent(RplComponent));
			entityRplID = rplCmp.Id();
			writer.WriteRplId(entityRplID);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RplLoad(ScriptBitReader reader)
	{
		RplId entityRplID;
		RplComponent rplComp;
		IEntity ent;
		int count;
		SCR_CampaignBuildingProviderComponent providerComponent;
		reader.ReadInt(count);

		for (int i = 0; i < count; i++)
		{
			reader.ReadRplId(entityRplID);
			if (!entityRplID.IsValid())
				continue;

			rplComp = RplComponent.Cast(Replication.FindItem(entityRplID));
			if (!rplComp)
			{
				m_aProvidersRplIds.Insert(entityRplID);
				continue;
			}

			ent = rplComp.GetEntity();
			if (!ent)
				continue;

			providerComponent = SCR_CampaignBuildingProviderComponent.Cast(ent.FindComponent(SCR_CampaignBuildingProviderComponent));
			AddProviderEntityEditorComponent(providerComponent);
		}

		if (!m_aProvidersRplIds.IsEmpty())
			SCR_CampaignBuildingProviderComponent.GetOnProviderCreated().Insert(SetProviderFromRplID);

		if (!m_aProvidersComponents.IsEmpty())
			GetOnProviderChanged().Invoke(m_aProvidersComponents[0]);

		return true;
	}
}
