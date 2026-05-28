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
	protected RplComponent m_RplComponent;
	SCR_CampaignBuildingBuildUserAction m_BuildAction;
	
	protected SCR_GadgetManagerComponent m_GadgetManager;

	protected const string DISMANTLE_ALL_BUILDINGS = "#AR-FactionCommander_DismantleAllBuildings";
	protected const string IN_USE = "#AR-UserAction_Blocked_InUseByOther";
	protected const string OCCUPIED = "#AR-UserAction_SeatOccupied";
	protected const string ENEMY_PRESENCE = "#AR-Campaign_Action_ShowBuildPreviewEnemyPresence";
	protected const int TEMPORARY_BLOCKED_ACCESS_RESET_TIME = 2;
	protected const int ALLOWED_PLAYER_DISTANCE_SQ = 10000; //! 100m

	//------------------------------------------------------------------------------------------------
	protected override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_RootEntity = pOwnerEntity.GetRootParent();
				
		m_CompositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(m_RootEntity.FindComponent(SCR_CampaignBuildingCompositionComponent));
		m_EditableEntity = SCR_EditableEntityComponent.Cast(m_RootEntity.FindComponent(SCR_EditableEntityComponent));
		m_LayoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignBuildingLayoutComponent));
		m_RplComponent = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
					
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

		m_User = pUserEntity;
		ToggleCompartmentAccess(GetOwner(), m_User);
		if (!SCR_CharacterHelper.IsPlayerOrAIOwner(character))
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

		super.OnActionStart(pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_User = null;
		ToggleCompartmentAccess(GetOwner(), null);

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		if (!SCR_CharacterHelper.IsPlayerOrAIOwner(character))
			return;

		CancelPlayerAnimation(character); // when player stops holding use action button
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to lock or unlock all compartments by reserving them for the player who performs the action
	//! \param[in] object which compratments and compartments of its children should be blocked
	//! \param[in] blockingUser for whom the compartments should be reserved, null if they should be unlocked
	protected void ToggleCompartmentAccess(notnull IEntity object, IEntity blockingUser = null)
	{
		Turret turret = Turret.Cast(object);
		if (turret)
		{
			SCR_BaseCompartmentManagerComponent compartmentMgr = SCR_BaseCompartmentManagerComponent.Cast(turret.FindComponent(SCR_BaseCompartmentManagerComponent));
			if (!compartmentMgr)
				return;
	
			array<BaseCompartmentSlot> outCompartments = {};
			if (compartmentMgr.GetCompartments(outCompartments) < 1)
				return;
	
			foreach (BaseCompartmentSlot compartment : outCompartments)
			{
				compartment.SetReserved(blockingUser);
			}

			return;
		}

		IEntity child = object.GetChildren();
		while (child)
		{
			ToggleCompartmentAccess(child, blockingUser);
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CancelPlayerAnimation(notnull IEntity entity)
	{		
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character)
			return;
		
		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController)
			return;

		CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
		CharacterCommandHandlerComponent cmdHandler = pAnimationComponent.GetCommandHandler();
		cmdHandler.FinishItemUse(true);
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if (!m_RplComponent || !m_RootEntity)
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CancelPlayerAnimation(character); // if player finished the action, then lets not wait for it to do all the RPCing, and end the animation now

		SCR_CampaignBuildingCompositionComponent buildingComponent = SCR_CampaignBuildingCompositionComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (buildingComponent)
		{
			if (!System.IsConsoleApp())
				buildingComponent.SetCanPlaySoundOnDeletion(true);
		}
		else if (!pOwnerEntity.FindComponent(SCR_CampaignBuildingLayoutComponent))
		{
			// Not buildable composition
			return;
		}

		if (m_RplComponent.IsProxy())
			return;

		if (character.GetCharacterController().GetLifeState() != ECharacterLifeState.ALIVE)
			return;

		PlayerManager playerMgr = GetGame().GetPlayerManager();
		int playerId = playerMgr.GetPlayerIdFromControlledEntity(character);
		if (!playerMgr.IsPlayerConnected(playerId))
			return;

		if (vector.DistanceSqXZ(m_RootEntity.GetOrigin(), character.GetOrigin()) > ALLOWED_PLAYER_DISTANCE_SQ)
			return;
		
		if (HasCompositionLabel())
			TryToSendNotification(pOwnerEntity, character);
		
		PrintFormat("Player %1 dismantled composition %2", SCR_PlayerIdentityUtils.GetPlayerLogInfo(playerId), m_RootEntity, level:LogLevel.DEBUG);

		SCR_CampaignMilitaryBaseComponent base;
		if (IsHQService())
		{
			if (CanBaseBeDisassembled(character))
				base = GetBase();
			else
				return;
		}

		DeleteComposition(m_RootEntity, character);

		// deleting base if it is possible
		if (base)
			DeleteBase(base, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteComposition(notnull IEntity composition, notnull SCR_ChimeraCharacter character)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(composition.GetRootParent().FindComponent(SCR_EditableEntityComponent));
		if (editableEntity)
		{
			editableEntity.Delete(true, true);
		}
		else
		{
			SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
			if (aiWorld)
			{
				array<ref Tuple2<vector, vector>> areas = {}; // min, max
				array<bool> redoAreas = {};
				aiWorld.GetNavmeshRebuildAreas(composition.GetRootParent(), areas, redoAreas);
				GetGame().GetCallqueue().CallLater(aiWorld.RequestNavmeshRebuildAreas, 1000, false, areas, redoAreas); //--- Called *before* the entity is deleted with a delay, ensures the regeneration doesn't accidentaly get anything from the entity prior to full destruction
			}

			RplComponent.DeleteRplEntity(composition, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteBase(notnull SCR_CampaignMilitaryBaseComponent base, int userPlayerId)
	{
		Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(userPlayerId);
		if (!playerFaction)
			return;

		// check if is exists dismantle task on this base for player faction
		SCR_DismantleCampaignMilitaryBaseTaskEntity task = SCR_DismantleCampaignMilitaryBaseTaskEntity.Cast(SCR_CampaignTaskHelper.GetTaskOnBase(base, playerFaction, SCR_DismantleCampaignMilitaryBaseTaskEntity));
		if (!task)
			return;

		// check if the player is assigned to task
		if (!task.IsTaskAssignedTo(SCR_TaskExecutor.FromPlayerID(userPlayerId)))
			return;

		PrintFormat("Base:%1 was dismantled by playerId:%2", base.GetBaseNameUpperCase(), userPlayerId, level:LogLevel.DEBUG);

		RplComponent.DeleteRplEntity(base.GetOwner(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TryToSendNotification(notnull IEntity pOwnerEntity, notnull SCR_ChimeraCharacter disassemblingCharacter)
	{
		if (!m_CompositionComponent)
			return;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(disassemblingCharacter);
			
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

		SCR_MilitaryBaseComponent base = bases[0];
		int callsign = base.GetCallsign();
		if (callsign == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
			return;
		
		Faction baseFaction = base.GetFaction();
		SCR_Faction characterFaction = SCR_Faction.Cast(disassemblingCharacter.GetFaction());
		if (characterFaction != baseFaction)
			return;

		RplComponent rplComponent = RplComponent.Cast(m_EditableEntity.GetOwner().FindComponent(RplComponent));
		if (!rplComponent)
			return;

		SCR_NotificationsComponent.SendToFaction(characterFaction, true, ENotification.EDITOR_SERVICE_DISASSEMBLED, playerId, rplComponent.Id(), callsign);
	}
	
	//------------------------------------------------------------------------------------------------
	// Check if the editable entity component has a specific label.
	protected bool HasCompositionLabel(EEditableEntityLabel label = EEditableEntityLabel.TRAIT_SERVICE)
	{
		if (!m_EditableEntity)
			return false;
		
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(m_EditableEntity.GetInfo());
		return editableEntityUIInfo && editableEntityUIInfo.HasEntityLabel(label);
	}
		
	//------------------------------------------------------------------------------------------------
	// The user action is shown when the preview is visible - means player has a building tool.
	override bool CanBeShownScript(IEntity user)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		if (m_bSameFactionDisassembleOnly && !IsPlayerFactionSame(character))
			return false;

		if (!m_GadgetManager)
		{
			m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(character);
			
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
		if (IsHQService() && !CanBaseBeDisassembled(character))
			return false;
		
		// The user action is on entity with composition component, show it if the composition is spawned.
		if (GetOwner() == GetOwner().GetRootParent())
			return m_bCompositionSpawned;
		
		if (m_BuildAction && !m_BuildAction.IsShown())
			return false;
		
		return m_LayoutComponent;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_EditableEntity)
			return false;

		SCR_UIInfo uiInfo = m_EditableEntity.GetInfo();
		if (!uiInfo)
			return false;

		UIInfo actionUiInfo = GetUIInfo();
		if (!actionUiInfo)
			return false;

		outName = WidgetManager.Translate(actionUiInfo.GetDescription(), uiInfo.GetName());
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets a new gadget manager. Controlled by an event when the controlled entity has changed.
	protected void SetNewGadgetManager(IEntity from, IEntity to)
	{
		m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(to);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		if (m_User && m_User != character)
		{
			SetCannotPerformReason(IN_USE);
			return false;
		}

		SCR_Faction scriptedFaction = SCR_Faction.Cast(character.GetFaction());
		if (scriptedFaction)
		{
			SCR_ECharacterRank playerRank = SCR_CharacterRankComponent.GetCharacterRank(user);

			if (scriptedFaction.GetRanks().IsRankRenegade(playerRank))
			{
				SetCannotPerformReason(SCR_CharacterRankComponent.GetRankName(user, playerRank));
				return false;
			}
		}
		
		if (m_bCompositionSpawned && !m_bTurretCollected)
		{
			m_bTurretCollected = true;
			GetAllTurretsInComposition(GetOwner());
		}
		
		// If the editor manager doesn't exists, try to get one and set as for an example when connecting to a server with build compositions, the editor manager doesn't exist when the user action inicialized.
		if (!m_EditorManager)
			SetEditorManager();
		
		if (!m_EditorManager || m_EditorManager.IsOpened())
			return false;
				
		if (m_bTurretCollected)
		{
			BaseCompartmentManagerComponent comaprtmentMgrComp;
			array<BaseCompartmentSlot> outCompartments = {};
			foreach (SCR_EditableVehicleComponent editableVehicle : m_EditableVehicle)
			{
				if (!editableVehicle)
					continue;
	
				comaprtmentMgrComp = BaseCompartmentManagerComponent.Cast(editableVehicle.GetOwner().FindComponent(BaseCompartmentManagerComponent));
				if (!comaprtmentMgrComp)
					continue;
	
				outCompartments.Clear();
				comaprtmentMgrComp.GetCompartments(outCompartments);
				foreach (BaseCompartmentSlot compartment : outCompartments)
				{
					if (!compartment.IsOccupied())
						continue;

					SetCannotPerformReason(OCCUPIED);
					return false;
				}
			}
		}
		
		
		if (IsHQService())
		{
			if (CanBaseBeDisassembled(character))
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

		if (!m_bDisassembleOnlyWhenCapturing || IsPlayerFactionSame(character))
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
		
		Faction playerFaction = character.GetFaction();
		
		return playerFaction && playerFaction == m_BaseComponent.GetCapturingFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Search for first instance of building action, as we can safely assume there is only one per entity.
	protected void GetBuildingAction()
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
	protected void SetEditorManager()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;

		m_EditorManager = core.GetEditorManager();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check the hierarchy of the composition for any turret and make a list of them.
	protected void GetAllTurretsInComposition(IEntity object)
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
	protected void OnCompositionSpawned(bool compositionSpawned)
	{
		m_bCompositionSpawned = compositionSpawned;
		if (m_CompositionComponent)
			m_CompositionComponent.GetOnCompositionSpawned().Remove(OnCompositionSpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get call once the provider is set. 
	protected void CacheFactionAffiliationComponent()
	{
		IEntity provider = m_CompositionComponent.GetProviderEntity();
		if (!provider)
			return;
		
		m_FactionComponent = FactionAffiliationComponent.Cast(provider.FindComponent(FactionAffiliationComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get building tool entity
	protected IEntity GetBuildingTool(notnull IEntity ent)
	{
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(ent);
		if (!gadgetManager)
			return null;
		
		return gadgetManager.GetHeldGadget();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is user faction same as the composition one.
	bool IsPlayerFactionSame(notnull SCR_ChimeraCharacter user)
	{
		if (!m_FactionComponent)
 			return true;

		Faction playerFaction = user.GetFaction();

		return playerFaction && m_FactionComponent.GetAffiliatedFaction() == playerFaction;
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

		SCR_CampaignMilitaryBaseManager baseMgr = campaign.GetBaseManager();
		if (!baseMgr)
			return null;

		IEntity owner = GetOwner();
		if (!owner)
			return null;

		return baseMgr.FindClosestBase(owner.GetOrigin());
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
	protected bool CanBaseBeDisassembled(notnull SCR_ChimeraCharacter disassemblingCharacter)
	{
		SCR_CampaignMilitaryBaseComponent campaignBase = GetBase();
		if (!campaignBase)
			return true;

		Faction playerFaction = disassemblingCharacter.GetFaction();
		if (!playerFaction)
			return false;

		// check if is exists dismantle task on this base for player faction
		SCR_DismantleCampaignMilitaryBaseTaskEntity task = SCR_DismantleCampaignMilitaryBaseTaskEntity.Cast(SCR_CampaignTaskHelper.GetTaskOnBase(campaignBase, playerFaction, SCR_DismantleCampaignMilitaryBaseTaskEntity));
		if (!task)
			return false;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(disassemblingCharacter);
		// check if the player is assigned to task
		if (!task.IsTaskAssignedTo(SCR_TaskExecutor.FromPlayerID(playerId)))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Check if the editable entity component has a service trait set.
	protected bool HasCompositionLabel(notnull SCR_CampaignBuildingCompositionComponent composition, EEditableEntityLabel label = EEditableEntityLabel.TRAIT_SERVICE)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(composition.GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!editableEntity)
			return false;

		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(editableEntity.GetInfo());
		return editableEntityUIInfo && editableEntityUIInfo.HasEntityLabel(label);
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
			if (composition && HasCompositionLabel(composition))
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
		if (!m_User)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(m_User);
		if (!SCR_CharacterHelper.IsPlayerOrAIOwner(character))
			return;

		CancelPlayerAnimation(character); // this needs to be here in case f.e. a GM deletes the composition while it is being dismantled
	}
}
