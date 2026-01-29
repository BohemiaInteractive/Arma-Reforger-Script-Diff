class SCR_CampaignBuildingDisassemblyUserAction : ScriptedUserAction
{	
	protected SCR_CampaignBuildingLayoutComponent m_LayoutComponent;
	protected SCR_CampaignBuildingCompositionComponent m_CompositionComponent;
	protected SCR_EditableEntityComponent m_EditableEntity;
	protected SCR_EditorManagerEntity m_EditorManager;
	protected FactionAffiliationComponent m_FactionComponent;
	protected SCR_MilitaryBaseComponent m_BaseComponent;
	protected ref array<SCR_EditableVehicleComponent> m_EditableVehicle = {};
	protected SCR_CampaignBuildingProviderComponent m_MasterProviderComponent;
	protected bool m_bCompositionSpawned;
	protected bool m_bTurretCollected;
	protected IEntity m_RootEntity;
	protected IEntity m_User;
	protected bool m_bDisassembleOnlyWhenCapturing = false;
	protected bool m_bSameFactionDisassembleOnly = false;
	protected bool m_bTemporarilyBlockedAccess;
	protected bool m_bAccessCanBeBlocked;
	protected WorldTimestamp m_ResetTemporaryBlockedAccessTimestamp;
	SCR_CampaignBuildingBuildUserAction m_BuildAction;
	SCR_FactionManager m_FactionManager;
	
	protected SCR_GadgetManagerComponent m_GadgetManager;

	protected const string DISMANTLE_ALL_BUILDINGS = "#AR-FactionCommander_DismantleAllBuildings";
	protected const string ENEMY_PRESENCE = "#AR-Campaign_Action_ShowBuildPreviewEnemyPresence";
	protected const int TEMPORARY_BLOCKED_ACCESS_RESET_TIME = 2;

	//------------------------------------------------------------------------------------------------
	protected override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_RootEntity = pOwnerEntity.GetRootParent();
				
		m_CompositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(m_RootEntity.FindComponent(SCR_CampaignBuildingCompositionComponent));
		m_EditableEntity = SCR_EditableEntityComponent.Cast(m_RootEntity.FindComponent(SCR_EditableEntityComponent));
		m_LayoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignBuildingLayoutComponent));
		m_FactionManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
					
		GetBuildingAction();
		SetEditorManager();
		
		if (m_CompositionComponent && GetOwner() == GetOwner().GetRootParent())
		{
			m_CompositionComponent.GetOnCompositionSpawned().Insert(OnCompositionSpawned);
			
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
				return;
	
			SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
			if (!buildingManagerComponent)
				return;
			
			m_bSameFactionDisassembleOnly = buildingManagerComponent.CanDisassembleSameFactionOnly();
			m_bDisassembleOnlyWhenCapturing = buildingManagerComponent.CanDisassembleOnlyWhenCapturing();
			
			if (m_bSameFactionDisassembleOnly || m_bDisassembleOnlyWhenCapturing)
				m_CompositionComponent.GetOnBuilderSet().Insert(CacheFactionAffiliationComponent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		CharacterControllerComponent charController = character.GetCharacterController();
		if (charController)
		{
			
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			int itemActionId = pAnimationComponent.BindCommand("CMD_Item_Action");

			ItemUseParameters params = new ItemUseParameters();
			params.SetEntity(GetBuildingTool(pUserEntity));
			params.SetAllowMovementDuringAction(false);
			params.SetKeepInHandAfterSuccess(true);
			params.SetCommandID(itemActionId);
			params.SetCommandIntArg(2);

			charController.TryUseItemOverrideParams(params);
		}
		
		m_User = pUserEntity;
		
		super.OnActionStart(pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_User = null;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		CharacterControllerComponent charController = character.GetCharacterController();
		if (charController)
		{
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			int itemActionId = pAnimationComponent.BindCommand("CMD_Item_Action");
			CharacterCommandHandlerComponent cmdHandler = pAnimationComponent.GetCommandHandler();
			if (cmdHandler)
				cmdHandler.FinishItemUse(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CancelPlayerAnimation(notnull IEntity entity)
	{		
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character)
			return;
		
		CharacterControllerComponent charController = character.GetCharacterController();
		if (charController)
		{
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			CharacterCommandHandlerComponent cmdHandler = pAnimationComponent.GetCommandHandler();
			cmdHandler.FinishItemUse(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{			
		bool isHQService = IsHQService();
		bool canBeBaseDisassembled = CanBeBaseDisassembled(pUserEntity);
		SCR_CampaignMilitaryBaseComponent base = GetBase();

		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent || !m_RootEntity)
			return;
		
		if (HasCompositionService())
			TryToSendNotification(pOwnerEntity, pUserEntity, networkComponent);
		
		SCR_CampaignBuildingCompositionComponent buildingComponent = SCR_CampaignBuildingCompositionComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (buildingComponent)
			buildingComponent.SetCanPlaySoundOnDeletion(true);
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);

		networkComponent.DeleteCompositionByUserAction(m_RootEntity, playerId);

		// deleting base if it is possible
		if (!isHQService || !canBeBaseDisassembled)
			return;

		if (base)
			networkComponent.DeleteBaseByUserAction(base.GetOwner(), playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	void TryToSendNotification(IEntity pOwnerEntity, IEntity pUserEntity, notnull SCR_CampaignBuildingNetworkComponent networkComponent)
	{
		if (!m_CompositionComponent)
			return;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
			
		IEntity provider = m_CompositionComponent.GetProviderEntity();
		if (!provider)
			return;
			
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;
			
		array<SCR_MilitaryBaseComponent> bases = {};
		providerComponent.GetBases(bases);
		if (bases.IsEmpty())
			return;
			
		int callsign = bases[0].GetCallsign();
		if (callsign == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
			return;
		
		Faction baseFaction = bases[0].GetFaction();
		Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(playerId);
		if (playerFaction != baseFaction)
			return;

		networkComponent.SendDeleteNotification(m_EditableEntity.GetOwner(), playerId, callsign);
	}
	
	//------------------------------------------------------------------------------------------------
	// Check if the editable entity component has a service trait set.
	bool HasCompositionService()
	{
		if (!m_EditableEntity)
			return false;
		
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(m_EditableEntity.GetInfo());
		return editableEntityUIInfo && editableEntityUIInfo.HasEntityLabel(EEditableEntityLabel.TRAIT_SERVICE);
	}
		
	//------------------------------------------------------------------------------------------------
	// The user action is shown when the preview is visible - means player has a building tool.
	override bool CanBeShownScript(IEntity user)
	{
		if (m_bSameFactionDisassembleOnly && !IsPlayerFactionSame(user))
			return false;
				
		if (!m_GadgetManager)
		{
			m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(user);
			
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.m_OnControlledEntityChanged.Insert(SetNewGadgetManager);
			
			return false;
		}
					
		if (!SCR_CampaignBuildingGadgetToolComponent.Cast(m_GadgetManager.GetHeldGadgetComponent()))
			return false;

		if (!CanBeDisassembled())
			return false;

		// checks only if it is HQ service
		if (IsHQService() && !CanBeBaseDisassembled(user))
			return false;
		
		// The user action is on entity with composition component, show it if the composition is spawned.
		if (GetOwner() == GetOwner().GetRootParent())
			return m_bCompositionSpawned;
		
		if (m_BuildAction && !m_BuildAction.IsShown())
			return false;
		
		return m_LayoutComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets a new gadget manager. Controlled by an event when the controlled entity has changed.
	void SetNewGadgetManager(IEntity from, IEntity to)
	{
		m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(to);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{ 					
		if (m_FactionManager && m_FactionManager.IsRankRenegade(SCR_CharacterRankComponent.GetCharacterRank(user)))
		{
			IEntity playerEntity = SCR_PlayerController.GetLocalMainEntity();
			if (!playerEntity)
				return false;
			
			SCR_ECharacterRank playerRank = SCR_CharacterRankComponent.GetCharacterRank(playerEntity);						
			SetCannotPerformReason(SCR_CharacterRankComponent.GetRankName(playerEntity, playerRank));
			return false;
		}
		
		if (m_bCompositionSpawned && !m_bTurretCollected)
			GetAllTurretsInComposition();
		
		// If the editor manager doesn't exists, try to get one and set as for an example when connecting to a server with build compositions, the editor manager doesn't exist when the user action inicialized.
		if (!m_EditorManager)
			SetEditorManager();
		
		if (!m_EditorManager || m_EditorManager.IsOpened())
			return false;
				
		array<CompartmentAccessComponent> crewCompartmentAccess = {};
		
		foreach (SCR_EditableVehicleComponent editableVehicle : m_EditableVehicle)
		{
			if (editableVehicle && editableVehicle.GetCrew(crewCompartmentAccess, false) != 0)
				return false;
		}
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(user);
		if (!gadgetManager)
			return false;
		
		SCR_GadgetComponent gadgetComponent = gadgetManager.GetHeldGadgetComponent();
		if (!gadgetComponent)
			return false;
		
		if (gadgetComponent.GetMode() != EGadgetMode.IN_HAND)
			return false;
		
		if (IsHQService())
		{
			if (CanBeBaseDisassembled(user))
			{
				if (HasBaseCompositionsAnyService())
				{
					SetCannotPerformReason(DISMANTLE_ALL_BUILDINGS);

					return false;
				}
			}
			else
			{
				return false;
			}
		}

		if (m_bAccessCanBeBlocked)
		{
			SetTemporaryBlockedAccess();

			if (m_bTemporarilyBlockedAccess)
			{
				SetCannotPerformReason(ENEMY_PRESENCE);
				return false;
			}
		}

		if (!m_bDisassembleOnlyWhenCapturing || IsPlayerFactionSame(user))
			return true;
		
		if (!m_BaseComponent)
		{
			if (!m_CompositionComponent)
				return false;
			
			IEntity provider = m_CompositionComponent.GetProviderEntity();
			if (!provider)
				return false;
			
			SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
			if (!providerComponent)
				return false;
			
			array<SCR_MilitaryBaseComponent> bases = {};
			providerComponent.GetBases(bases);
			if (bases.IsEmpty())
				return false;
			
			m_BaseComponent = bases[0];
		}
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(playerId);
		
		return playerFaction == m_BaseComponent.GetCapturingFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Search for first instance of building action, as we can safely assume there is only one per entity.
	void GetBuildingAction()
	{
		BaseActionsManagerComponent baseActionManager = GetActionsManager();
		if (!baseActionManager)
			return;

		array<BaseUserAction> actions = {};
		baseActionManager.GetActionsList(actions); 
		
		foreach (BaseUserAction action : actions)
		{
			m_BuildAction = SCR_CampaignBuildingBuildUserAction.Cast(action);
			if (m_BuildAction)
				break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEditorManager()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;

		m_EditorManager = core.GetEditorManager();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check the hierarchy of the composition for any turret and make a list of them.
	void GetAllTurretsInComposition()
	{
		if (!m_EditableEntity)
			return;

		m_bTurretCollected = true;
		set<SCR_EditableEntityComponent> editableEntities = new set<SCR_EditableEntityComponent>();
		m_EditableEntity.GetChildren(editableEntities);
				
		foreach (SCR_EditableEntityComponent ent : editableEntities)
		{
			SCR_EditableVehicleComponent editableVehicle = SCR_EditableVehicleComponent.Cast(ent);
			if (editableVehicle)
				m_EditableVehicle.Insert(editableVehicle);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get call once the composition is fully spawned
	void OnCompositionSpawned(bool compositionSpawned)
	{
		m_bCompositionSpawned = compositionSpawned;
		if (m_CompositionComponent)
			m_CompositionComponent.GetOnCompositionSpawned().Remove(OnCompositionSpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get call once the provider is set. 
	void CacheFactionAffiliationComponent()
	{
		IEntity provider = m_CompositionComponent.GetProviderEntity();
		if (!provider)
			return;
		
		m_FactionComponent = FactionAffiliationComponent.Cast(provider.FindComponent(FactionAffiliationComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get building tool entity
	IEntity GetBuildingTool(notnull IEntity ent)
	{
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(ent);
		if (!gadgetManager)
			return null;
		
		return gadgetManager.GetHeldGadget();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is user faction same as the composition one.
	bool IsPlayerFactionSame(notnull IEntity user)
	{
		if (!m_FactionComponent)
 			return true;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(playerId);
		
		return m_FactionComponent.GetAffiliatedFaction() == playerFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! GetNetworkManager
	protected SCR_CampaignBuildingNetworkComponent GetNetworkManager()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;

		return SCR_CampaignBuildingNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignBuildingNetworkComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsHQService()
	{
		SCR_GameModeCampaign campaignGameMode = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (!campaignGameMode || !m_EditableEntity)
			return false;

		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(m_EditableEntity.GetInfo(GetOwner()));
		if (!editableEntityUIInfo)
			return false;

		array<EEditableEntityLabel> entityLabels = {};
		editableEntityUIInfo.GetEntityLabels(entityLabels);
		if (entityLabels.Contains(EEditableEntityLabel.SERVICE_HQ))
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CampaignMilitaryBaseComponent GetBase()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return null;

		return campaign.GetBaseManager().FindClosestBase(GetOwner().GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanBeDisassembled()
	{
		// is not headquarter building, can be disassembled
		if (!IsHQService())
			return true;

		SCR_CampaignMilitaryBaseComponent campaignBase = GetBase();
		if (!campaignBase)
			return true;

		if (campaignBase.IsHQ() || campaignBase.IsControlPoint())
			return false;

		SCR_ECampaignBaseType baseType = campaignBase.GetType();
		if (baseType == SCR_ECampaignBaseType.RELAY || baseType == SCR_ECampaignBaseType.SOURCE_BASE)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanBeBaseDisassembled(IEntity playerEntity)
	{
		if (!playerEntity)
			return false;

		SCR_CampaignMilitaryBaseComponent campaignBase = GetBase();
		if (!campaignBase)
			return true;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);
		Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(playerId);
		if (!playerFaction)
			return false;

		// check if is exists dismantle task on this base for player faction
		SCR_DismantleCampaignMilitaryBaseTaskEntity task = SCR_DismantleCampaignMilitaryBaseTaskEntity.Cast(SCR_CampaignTaskHelper.GetTaskOnBase(campaignBase, playerFaction, SCR_DismantleCampaignMilitaryBaseTaskEntity));
		if (!task)
			return false;

		// check if the player is assigned to task
		if (!task.IsTaskAssignedTo(SCR_TaskExecutor.FromPlayerID(playerId)))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Check if the editable entity component has a service trait set.
	protected bool HasCompositionService(SCR_CampaignBuildingCompositionComponent composition)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(composition.GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!editableEntity)
			return false;

		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(editableEntity.GetInfo());
		return editableEntityUIInfo && editableEntityUIInfo.HasEntityLabel(EEditableEntityLabel.TRAIT_SERVICE);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasBaseCompositionsAnyService()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return false;

		SCR_CampaignMilitaryBaseComponent campaignBase = GetBase();
		if (!campaignBase)
			return false;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(campaign.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return false;

		array<SCR_CampaignBuildingCompositionComponent> compositions = {};
		int compositionCount = buildingManagerComponent.GetBuildingCompositions(campaignBase, compositions);
		foreach (SCR_CampaignBuildingCompositionComponent composition : compositions)
		{
			if (composition && HasCompositionService(composition))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Set temporary blocked access.
	void SetTemporaryBlockedAccess()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		WorldTimestamp currentServerTimestamp = world.GetServerTimestamp();
		if (!currentServerTimestamp.Greater(m_ResetTemporaryBlockedAccessTimestamp))
			return;

		m_bTemporarilyBlockedAccess = false;

		// cache master provider component
		if (!m_MasterProviderComponent)
		{
			SCR_CampaignMilitaryBaseComponent campaignBase = GetBase();
			if (!campaignBase)
				return;

			array<SCR_CampaignBuildingProviderComponent> providers = {};
			campaignBase.GetBuildingProviders(providers);

			foreach (SCR_CampaignBuildingProviderComponent provider : providers)
			{
				if (!provider.IsMasterProvider())
					continue;

				m_MasterProviderComponent = provider;
				break;
			}
		}

		if (!m_MasterProviderComponent)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		array<int> outPlayers = {};
		playerManager.GetPlayers(outPlayers);

		float buildingRadiusSq = m_MasterProviderComponent.GetBuildingRadius() * m_MasterProviderComponent.GetBuildingRadius();

		IEntity playerEntity;
		SCR_CharacterDamageManagerComponent charDamageManager;
		foreach (int playerID : outPlayers)
		{
			playerEntity = playerManager.GetPlayerControlledEntity(playerID);
			if (!playerEntity)
				continue;

			SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(playerEntity);
			if (!char)
				continue;

			charDamageManager = SCR_CharacterDamageManagerComponent.Cast(char.FindComponent(SCR_CharacterDamageManagerComponent));
			if (!charDamageManager || charDamageManager.GetState() == EDamageState.DESTROYED)
				continue;

			if (!m_MasterProviderComponent.IsEnemyFaction(char))
				continue;

			if (vector.DistanceSqXZ(GetOwner().GetOrigin(), playerEntity.GetOrigin()) <= buildingRadiusSq)
			{
				m_bTemporarilyBlockedAccess = true;
				m_ResetTemporaryBlockedAccessTimestamp = currentServerTimestamp.PlusSeconds(TEMPORARY_BLOCKED_ACCESS_RESET_TIME);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Destructor
	void ~SCR_CampaignBuildingDisassemblyUserAction()
	{
		if (m_User)
			CancelPlayerAnimation(m_User);
	}
}
